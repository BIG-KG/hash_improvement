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

error_t test_string(uint32_t (*hashfunction)(hash_value_t, lst_hash_table_t *), char **stringArray, 
                                                                                uint32_t numOfStrings)
{
    lst_hash_table_t table = {};

    table.cmpfunction = hasing_compare_string;
    table.hashfunction = hashfunction;
    table.rehashing = true;

    table.allocatedSegment = (lst_hash_node_t *)calloc(numOfStrings * 2, sizeof(lst_hash_node_t));
    table.allocatingSize = numOfStrings * 2;

    reinit_list_table(&table, TABLE_SIZE_PART1);
    
    uint32_t numOfActions = numOfStrings * 2;
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
                find_list_table(tmp, &table);
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
    //if(argc != 2) assert(0);

    char **stringArray = scanFile("./string.txt", &numOfValues);

    if(argc == 1 || !strcmp(argv[1], "0"))
        test_string(fnv1a_hash, stringArray, numOfValues);
    else
        test_string(fnv1a_hash_asm,     stringArray, numOfValues);
    

    return 0;
}