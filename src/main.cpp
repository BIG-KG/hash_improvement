#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

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
    return dup
}

error_t test_string(uint32_t (*hashfunction)(hash_value_t, lst_hash_table_t *), char **stringArray, 
                                                                                char  *numOfStrings)
{
    lst_hash_table_t testingTable = {};
    testingTable.cmpfunction = hasing_compare_string;
    testingTable.hashfunction = hashfunction;
    testingTable.rehashing = true;

    testingTable.allocatedSegment = (lst_hash_node_t *)calloc(numOfValues, sizeof(lst_hash_node_t));
    testingTable.allocatingSize = numOfValues;

    reinit_list_table(&testingTable, TABLE_SIZE_PART1);
    char *savingString = (char *)calloc(numOfValues * 21, sizeof(char));
    
    for(int i = 0; i < numOfValues; i++)
    {
        hash_value_t temp;
        temp.ptr = &savingString[i * 21];
        fscanf(inputFile, "%s", temp.ptr);
        add_to_list_table(&testingTable, temp);
        printf("add val%d \n", i);
    }

    fclose(inputFile);

    return ERR_SUCCESS;
}

char ** scanFile(char *filename)
{
    FILE *inputFile = fopen(filename, "r");

    int32_t numOfStrings = fscanf(inputFile, "%d", &numOfStrings);

    char **strings = (char **)calloc(numOfStrings, sizeof(char *));
    char buffer[21] = {};

    for(int i = 0; i < numOfStrings; i++)
    {
        fscanf(inputFile, "%s", buffer);
        strings[i] = strdup_my(buffer);
    }

    fclose(inputFile);

    return strings;
}

int main()
{
    char **stringArray = scanFile("./string.txt");

    test_string(hashing_crc32_string, stringArray);

    return 0;
}