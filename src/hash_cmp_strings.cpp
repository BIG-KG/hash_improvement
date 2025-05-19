#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <immintrin.h>
#include <SDL2/SDL.h> 

#include <immintrin.h>
#include <stdint.h>

int32_t hasing_compare_string_simd(hash_value_t s1Struct, hash_value_t s2Struct)
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

    }while(s1[-1] != '\0');           //если хотя бы один бит равен 0xFF то  _mm_testz_si128(cmpZero, cmpZero) даст 0 

    return 0;
}

int32_t hasing_compare_string(hash_value_t s1, hash_value_t s2)
{
    return strcmp((char *)s1.ptr, (char *)s2.ptr);
}