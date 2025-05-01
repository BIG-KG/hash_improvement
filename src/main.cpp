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
        action = rand() % 3;
        value  = rand() % numOfStrings;
        hash_value_t tmp;
        tmp.ptr = stringArray[value];
        //if(strcmp("tudqq", stringArray[value]) * strcmp("evymchc", stringArray[value]) * strcmp("jvculqp", stringArray[value])) continue;
        //else
        //{
        //printf("h %d %d %s\n", i, action, stringArray[value]);
        //}
        switch(action)
        {
            case 0:
                add_to_list_table(&table, tmp);
                break;
            case 1:
                //add_to_list_table(&table, tmp);
                //break;
                delete_from_list_table(&table, tmp);
                break;
            case 2:
                //add_to_list_table(&table, tmp);
                //break;
                find_list_table(tmp, &table);
                break;
        }

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
    char *buffer   = (char  *)calloc(numOFValues, 21 * sizeof(char));

    for(int i = 0; i < numOFValues; i++)
    {
        fscanf(inputFile, "%s", buffer + i * 21);
        strings[i] = buffer + i * 21;
    }

    fclose(inputFile);

    *numOfStrings = numOFValues;
    CALLGRIND_START_INSTRUMENTATION;
    return strings;
}

int main()
{
    uint32_t numOfValues = 0;

    char **stringArray = scanFile("./string.txt", &numOfValues);

    //printf("%d\n", numOfValues);

    test_string(hashing_crc32_string, stringArray, numOfValues);

    return 0;
}