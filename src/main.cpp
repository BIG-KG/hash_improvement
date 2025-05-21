#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <valgrind/callgrind.h>
#include <time.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <common.h>

extern "C" uint32_t hashing_crc32_string_asm(hash_value_t inputKey, lst_hash_table_t* table);

#include <lst_hash_cmp_functions.h>

const int BUFFER_SIZE = 512;

float test_string(char **stringArray, uint32_t numOfStrings                                                      )
{
    lst_hash_table_t table = {};
    uint32_t numOfActions = numOfStrings * 10;
    uint32_t action = 0, value = 0;
    srand(0);


    if(init_list_table(true,  &table, TABLE_SIZE_PART1, numOfActions)) return -1.0f;

    float start_time = clock();
    for(int i = 0; i < numOfActions; i++)
    {   
        action = rand() % 3;
        value  = rand() % numOfStrings;
        hash_value_t tmp;
        tmp.ptr = stringArray[value];
        CALLGRIND_TOGGLE_COLLECT;
        switch(action)
        {
            case 0:
            case 1:
                add_to_list_table(&table, tmp);
                break;

            #ifndef MY_UPGRADE_3
            case 2:
                find_list_table(tmp, &table);
                break;
            #else
            case 2:
                find_list_table_nasm(tmp, &table);
            #endif
        }
        CALLGRIND_TOGGLE_COLLECT;

    }
    float end_time = clock();

    destroy_list_table(&table);

    return (end_time - start_time) / CLOCKS_PER_SEC;
}


char **scanFile16bit(char *filename, uint32_t *numOfStrings)
{
    FILE *inputFile = fopen(filename, "r");
    if(!inputFile) return NULL;

    char buffer[512];
    uint32_t finalBufferSize = 0;
    uint32_t numOfStringsLocal = 0;
    while(fscanf(inputFile, "%s", buffer) != EOF)
    {
        finalBufferSize += (((strlen(buffer) + 1) / MINIMUM_STRING_LENGTH_STEP) + 1) * MINIMUM_STRING_LENGTH_STEP;
        numOfStringsLocal++;
    }

    printf("num of strings: %d\n", numOfStringsLocal);


    char *buffer16bit = (char  *)calloc(finalBufferSize, sizeof(char));
    char **strings =    (char **)calloc(numOfStringsLocal, sizeof(char *));
    if(!strings || !buffer16bit) return NULL;

    char *tmp = buffer16bit;
    fseek(inputFile, 0, SEEK_SET);


    for(int i = 0; i < numOfStringsLocal; i++)
    {
        fscanf(inputFile, "%s", tmp);
        strings[i] = tmp;
        tmp += (((strlen(tmp) + 1) / MINIMUM_STRING_LENGTH_STEP) + 1) * MINIMUM_STRING_LENGTH_STEP;
    }

    *numOfStrings = numOfStringsLocal;

    return strings;
}

int main(){
    uint32_t numOfValues = 0;
    char **stringArray = scanFile16bit("./string.txt", &numOfValues);
    
    if(stringArray == NULL) return 1;

    float resulTime = test_string(stringArray, numOfValues);

    free(stringArray[0]);
    free(stringArray);

    printf("Total time: %f\n", resulTime);
    
    return 0;
}