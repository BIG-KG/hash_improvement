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

error_t test_string(uint32_t (*hashfunction)(hash_value_t, lst_hash_table_t *), char *input_FileNameWthPath)
{
    FILE *inputFile = fopen(input_FileNameWthPath, "r");
    if(inputFile == NULL){
        printf("nossxs %s\n", input_FileNameWthPath);
        return ERR_SUCCESS;
    }

    uint32_t numOfValues = 0;
    fscanf(inputFile, "%d", &numOfValues);

    lst_hash_table_t testingTable = {};
    testingTable.cmpfunction = hasing_compare_string;
    testingTable.hashfunction = hashfunction;
    testingTable.rehashing = false;

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

int main()
{
    test_string(hashing_crc32_string, "./string.txt");

    return 0;
}