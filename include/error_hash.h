#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>

enum error_t
{
    ERR_SUCCESS,
    ERR_NULL_POINTER,
    ERR_NOT_FOUND,
    ERR_NO_MEMORY,
    ERR_FILE_OPENING_FAILED,
    ERR_OVERLOADED_HASH,
    ERR_ENDLESS_LOOP
};

#define VOID_RETURN_ERROR_NULL(check) if((check) == NULL) return;
#define NULL_RETURN_ERROR_NULL(check) if((check) == NULL) return NULL;
#define UINT_RETURN_ERROR_NULL(check) if((check) == NULL) return ERR_NULLPTR;

#endif // ERRORS_H

