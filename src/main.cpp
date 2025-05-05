#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <valgrind/callgrind.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <common.h>

extern "C" uint32_t fnv1a_hash_asm(hash_value_t inputKey, lst_hash_table_t* table);

#include <lst_hash_cmp_functions.h>

char *strdup_my(const char *str)
{
    char *dup = (char *)calloc(strlen(str), 1);
    if(dup == NULL)
        return NULL;

    strcpy(dup, str);
    return dup;
}

error_t test_string(uint32_t         (*hashfunction)(hash_value_t, lst_hash_table_t *),
                    int32_t          (*cmpFunction) (hash_value_t, hash_value_t),
                    lst_hash_node_t *(*findFunction)(hash_value_t targetValue, lst_hash_table_t *hashTable),
                    char **stringArray, uint32_t numOfStrings                                                      )
{
    lst_hash_table_t table = {};

    table.cmpFunction = cmpFunction;
    table.hashfunction = hashfunction;
    table.rehashing = true;

    table.allocatedSegment = (lst_hash_node_t *)calloc(numOfStrings * 30, sizeof(lst_hash_node_t));
    table.allocatingSize = numOfStrings * 30;

    table.findFunction = findFunction;

    reinit_list_table(&table, TABLE_SIZE_PART1);
    
    uint32_t numOfActions = numOfStrings * 30;
    uint32_t action = 0, value = 0;
    srand(0);

    for(int i = 0; i < numOfActions; i++)
    {   
        action = rand() % 4;
        value  = rand() % numOfStrings;
        hash_value_t tmp;
        tmp.ptr = stringArray[value];
        //printf("h %d %d %s______________________\n", i, action, stringArray[value]);
        CALLGRIND_TOGGLE_COLLECT;
        switch(action)
        {
            case 0:
            case 1:
                add_to_list_table(&table, tmp);
                break;
            case 2:
                delete_from_list_table(&table, tmp);
                break;
            case 3:
                table.findFunction(tmp, &table);
                break;
        }
        CALLGRIND_TOGGLE_COLLECT;

    }

    return ERR_SUCCESS;
}

char ** scanFile(char *filename, uint32_t *numOfStrings)
{
    CALLGRIND_STOP_INSTRUMENTATION;
    FILE *inputFile = fopen(filename, "r");

    int32_t numOFValues = 0;
    fscanf(inputFile, "%d", &numOFValues);

    char **strings = (char **)calloc(numOFValues,      sizeof(char *));
    char *buffer   = (char  *)calloc(numOFValues, 32 * sizeof(char));

    for(int i = 0; i < numOFValues; i++)
    {
        fscanf(inputFile, "%s", buffer + i * 32);
        strings[i] = buffer + i * 32;
    }

    fclose(inputFile);

    *numOfStrings = numOFValues;
    CALLGRIND_START_INSTRUMENTATION;
    return strings;
}

int main(int argc, char *argv[]){
    uint32_t numOfValues = 0;
    char **stringArray = scanFile("./string.txt", &numOfValues);


    uint32_t         (*hashfunction)(hash_value_t, lst_hash_table_t *) = fnv1a_hash;
    int32_t          (*cmpFunction) (hash_value_t, hash_value_t)       = hasing_compare_string;
    lst_hash_node_t *(*findFunction)(hash_value_t targetValue, lst_hash_table_t *hashTable) = find_list_table;
    

    if (argc == 4){
        if(!strcmp(argv[1], "1"))
            hashfunction = fnv1a_hash_asm;
        if(!strcmp(argv[2], "1"))
            cmpFunction = hasing_compare_string_simd;
        if(!strcmp(argv[3], "1"))
            findFunction = find_list_table_nasm;
    }

    test_string(hashfunction, cmpFunction, findFunction, stringArray, numOfValues);
    
    return 0;
}