#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <common.h>

#include <immintrin.h>
#include <SDL2/SDL.h> 

#include <lst_hash_cmp_functions.h>

error_t realloc_list_table(lst_hash_table_t *table);

extern "C" uint32_t hashing_crc32_string_asm(hash_value_t inputKey, lst_hash_table_t* table);

extern "C" int32_t hasing_compare_string_simd(hash_value_t s1Struct, hash_value_t s2Struct)
{
    char *s1 = (char *)s1Struct.ptr;
    char *s2 = (char *)s2Struct.ptr;

    do{
        __m128i *s1Vector = (__m128i *)s1;
        __m128i result = _mm_xor_si128(*(__m128i *)s1, *(__m128i *)s2);
        int cmp = _mm_testz_si128(result, result);
        if(!cmp) return 1;
        s1 += 16;
        s2 += 16;

    }while(s1[-1] != '\0');     //если хотя бы один бит равен 0xFF то  _mm_testz_si128(cmpZero, cmpZero) даст 0 

    return 0;
}

extern "C" int32_t hasing_compare_string(hash_value_t s1, hash_value_t s2)
{
    return strcmp((char *)s1.ptr, (char *)s2.ptr);
}

void crc32_init(uint32_t *crc32_table) {
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c >> 1) ^ (polynomial & -(c & 1));
        crc32_table[i] = c;
    }
}

extern "C" __attribute__((used)) uint32_t hashing_crc32_string(hash_value_t inputKey, lst_hash_table_t *table){
    static int startCounter = 0;
    static uint32_t crc32_table[256];

    if(startCounter == 0)
    {
        crc32_init(crc32_table);
        startCounter ++;
    }

    const uint8_t* bytes = (const uint8_t*)inputKey.ptr;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; bytes[i] != '\0' ; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[i]) & 0xFF];

    crc ^= 0xFFFFFFFF;

    return crc % table->tableSize;
}

lst_hash_node_t *find_list_table(hash_value_t targetValue, lst_hash_table_t *hashTable)
{
    #ifndef MY_UPGRADE_1
        uint32_t index = hashing_crc32_string(targetValue, hashTable);// % hashTable->tableSize;
    #else
        uint32_t index = hashing_crc32_string_asm(targetValue, hashTable);// % hashTable->tableSize;
    #endif
    lst_hash_node_t *head = hashTable->table[index];

    if (head == NULL)
    {
        return NULL;
    }
    

    #ifndef MY_UPGRADE_2
        while (!hasing_compare_string(head->value, targetValue) && head->next != NULL)
        {
            head = head->next;
        }
    #else
        while (!hasing_compare_string_simd(head->value, targetValue) && head->next != NULL)
        {
            head = head->next;
        }
    #endif
    
    #ifndef MY_UPGRADE_2
        if (hasing_compare_string(head->value, targetValue) != 0)
        {
            return NULL;
        }
    #else
        if (hasing_compare_string_simd(head->value, targetValue) != 0)
        {
            return NULL;
        }
    #endif

    return head;
}

__attribute__((optimize("O0")))
lst_hash_node_t *find_list_table_nasm(hash_value_t targetValue, lst_hash_table_t *hashTable)
{
    lst_hash_node_t *result = NULL;
    
__asm__ __volatile__ (
        "push %%rbp\n"
        "mov %%rsp, %%rbp\n"
        "and $-16, %%rsp\n"
    
        "mov %[targetValue], %%rdi\n"         // 1st arg = targetValue
        "mov %[hashTable], %%rsi\n"           // 2nd arg = hashTable
        #ifndef MY_UPGRADE_1
        "call hashing_crc32_string\n"                       // call hashfunction
        #else
        "call hashing_crc32_string_asm\n"
        #endif
    
        "mov 16(%[hashTable]), %%rdx\n"       // rdx = hashTable->table
        "mov (%%rdx,%%rax,8), %%rbx\n"        // rbx = hashTable->table[index]
    
        "test %%rbx, %%rbx\n"                 // if (head == NULL)
        "jnz .check\n"                        // goto .check
        "mov $0, %[result]\n"                 // result = NULL
        "jmp .end\n"                          // return
    
        ".Loop:\n"
        "mov (%%rbx), %%rbx\n"                // rbx = next
    
        ".check:\n"
        "mov %[targetValue], %%rdi\n"         // 1st arg = targetValue
        "mov 8(%%rbx), %%rsi\n"               // 2nd arg = value
        #ifndef MY_UPGRADE_2
        "call hasing_compare_string\n"                     // call cmpfunction
        #else
        "call hasing_compare_string_simd\n"
        #endif
        "test %%eax, %%eax\n"                 // eax == 0 if match
        "jnz .checkFalseNext\n"               // if not match, continue
        "mov %%rbx, %[result]\n"              // result = rbx
        "jmp .end\n"
    
        ".checkFalseNext:\n"
        "test %%eax, (%%rbx)\n"
        "jnz .Loop\n"
        "mov $0, %[result]\n"                 // result = NULL
        "jmp .end\n"
    
        ".end:\n"
        "mov %%rbp, %%rsp\n"
        "pop %%rbp\n"
        : [result] "=r" (result)
        : [cmpF] "r" (hasing_compare_string),
          [targetValue] "r" (targetValue),
          [hashTable] "r" (hashTable)
        : "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "cc", "memory"
    );

    return result;
}

lst_hash_node_t *give_next_node_pointer(lst_hash_table_t *table)
{
    if(table->deleteList != NULL)
    {
        lst_hash_node_t *free = table->deleteList;
        table->deleteList = table->deleteList->next;
        return free;
    }

    if(table->numberOfElements >= table->allocatingSize)
    {
        if(realloc_list_table(table) != ERR_SUCCESS) return NULL;
    }

    return table->allocatedSegment + table->allUsed++;
}

error_t realloc_list_table(lst_hash_table_t *table)
{
    lst_hash_node_t *oldNode = table->allocatedSegment;
    uint32_t newSize = next_prime (table->allocatingSize * 2);
    table->allocatedSegment = (lst_hash_node_t *) realloc(table->allocatedSegment, 
                                                             newSize * sizeof(lst_hash_node_t));
    if(table->allocatedSegment == NULL)
    {
        assert(0);
        table->allocatedSegment = oldNode;
        return ERR_NO_MEMORY;
    }

    for (lst_hash_node_t *i = table->allocatedSegment; i < table->allocatedSegment + table->allocatingSize; i++)
    {
        if(i->next != NULL)
            i->next += (table->allocatedSegment - oldNode) / sizeof(lst_hash_node_t);

        if(i->prev != NULL)
            i->prev += (table->allocatedSegment - oldNode) / sizeof(lst_hash_node_t);
    }

    table->allocatingSize = newSize; 
    
    return ERR_SUCCESS;
}

error_t add_to_list_table(lst_hash_table_t *table, hash_value_t value)
{

    #ifndef MY_UPGRADE_1
        uint32_t index = hashing_crc32_string(value, table);// % hashTable->tableSize;
    #else
        uint32_t index = hashing_crc32_string_asm(value, table);// % hashTable->tableSize;
    #endif
    lst_hash_node_t *previousNode = NULL;

    #ifndef MY_UPGRADE_3
        if(table->checkIfValueInTable == true){
            if(find_list_table(value, table) != NULL) return ERR_SUCCESS;
        }
    #else
        if(table->checkIfValueInTable == true){
            if(find_list_table_nasm(value, table) != NULL) return ERR_SUCCESS;
        }
    #endif

    lst_hash_node_t *newNode = give_next_node_pointer(table);
    newNode->value = value;
    newNode->next = table->table[index];
    newNode->isActive = true;
    table->table[index] = newNode;
    table->numberOfElements++;

    newNode->prev = NULL;
    if(newNode->next != NULL) newNode->next->prev = newNode;

    return check_coefficient_list_table(table);
}

error_t delete_from_list_table(lst_hash_table_t *table, hash_value_t value)
{
    lst_hash_node_t *previousNode = NULL, *targetNode = NULL;
    #ifndef MY_UPGRADE_3
        targetNode = find_list_table(value, table);
    #else
        targetNode = find_list_table_nasm(value, table);
    #endif

    if (targetNode == NULL)
    {
        return ERR_SUCCESS;
    }

    previousNode = targetNode->prev;

    if (previousNode == NULL)
    {

        #ifndef MY_UPGRADE_1
            table->table[hashing_crc32_string(value, table)] = targetNode->next;
        #else
            table->table[hashing_crc32_string_asm(value, table)] = targetNode->next;
        #endif

        return ERR_SUCCESS;
    }

    previousNode->next = targetNode->next;

    targetNode->next = table->deleteList;
    table->deleteList = targetNode;
    targetNode->isActive = false;
    table->numberOfElements--;

    return ERR_SUCCESS;   
}

error_t check_coefficient_list_table(lst_hash_table_t *table)
{
    if (table->tableSize == false) return ERR_SUCCESS;

    if ((float)table->numberOfElements / (float)table->tableSize >= MAX_LOAD_FACTOR)
    {
        printf("reallocating table_________________%d\n", table->tableSize);
        error_t error = ERR_SUCCESS;
        if(error = reinit_list_table(table, next_prime(table->tableSize * 2))) return error;

        table->numberOfElements = 0;
        table->rehashing = false;
        table->checkIfValueInTable = false;
        uint32_t currentUsed = table->allUsed;
        table->allUsed = 0;

        for (size_t i = 0; i < table->allocatingSize && i < currentUsed + 1; i++)
        {
            if(table->allocatedSegment[i].isActive) add_to_list_table(table, table->allocatedSegment[i].value);
        }
        
        table->deleteList = NULL;
        table->allUsed = table->numberOfElements;
        table->rehashing = true;
        table->checkIfValueInTable = true;
    }

    return ERR_SUCCESS;
}

error_t reinit_list_table(lst_hash_table_t *table, uint32_t size)
{   
    if(table == NULL) return ERR_NULL_POINTER;

    lst_hash_node_t **newTable = NULL;
    newTable = (lst_hash_node_t **)calloc(size, sizeof(lst_hash_node_t *));
    if(newTable == NULL) return ERR_NO_MEMORY;

    free(table->table);
    table->table = newTable;

    table->tableSize = size;

    table->hashingConst1 = next_prime(rand() % 1024);
    table->hashingConst2 = next_prime(rand() % 1024);

    return ERR_SUCCESS;
}


error_t init_list_table(bool rehashing, lst_hash_table_t *table, uint32_t size, uint32_t startNumOfNodes)
{
    table->allocatedSegment = (lst_hash_node_t *)calloc(startNumOfNodes, sizeof(lst_hash_node_t));
    if(table->allocatedSegment == NULL) return ERR_NO_MEMORY;
    table->allocatingSize = startNumOfNodes;

    if (reinit_list_table(table, size))
    {
        free(table->allocatedSegment);
        return ERR_NO_MEMORY;
    }

    table->rehashing    = rehashing;

    return ERR_SUCCESS;
}

error_t destroy_list_table(lst_hash_table_t *table)
{
    free(table->table);
    free(table->allocatedSegment);

    return ERR_SUCCESS;
}

