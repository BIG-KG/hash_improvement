#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <common.h>

error_t realloc_list_table(lst_hash_table_t *table);

lst_hash_node_t *find_list_table(hash_value_t targetValue, lst_hash_table_t *hashTable)
{
    uint32_t index = hashTable->hashfunction(targetValue, hashTable);// % hashTable->tableSize;
    lst_hash_node_t *head = hashTable->table[index];

    if (head == NULL)
    {
        return NULL;
    }

    while (!hashTable->cmpFunction(head->value, targetValue) && head->next != NULL)
    {
        head = head->next;
    }

    if (hashTable->cmpFunction(head->value, targetValue) != 0)
    {
        return NULL;
    }

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
    
        "mov 16(%[hashTable]), %%rbx\n"       // rbx = hashTable->hashfunction
        "mov %[targetValue], %%rdi\n"         // 1st arg = targetValue
        "mov %[hashTable], %%rsi\n"           // 2nd arg = hashTable
        "call *%%rbx\n"                     // call hashfunction
    
        "mov 24(%[hashTable]), %%rdx\n"       // rdx = hashTable->table
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
        "call *%[cmpF]\n"                     // call cmpfunction
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
        : [cmpF] "r" (hashTable->cmpFunction),
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
    assert(0);

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
    uint32_t index = table->hashfunction(value, table) % table->tableSize;
    lst_hash_node_t *previousNode = NULL;

    if(table->checkIfValueInTable == true){
        if(table->findFunction(value, table) != NULL) return ERR_SUCCESS;
    }

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
    targetNode = table->findFunction(value, table);

    if (targetNode == NULL)
    {
        return ERR_SUCCESS;
    }

    previousNode = targetNode->prev;

    if (previousNode == NULL)
    {
        table->table[table->hashfunction(value, table) % table->tableSize] = targetNode->next;
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


