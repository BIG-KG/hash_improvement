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

int32_t hasing_compare_string_simd(hash_value_t s1Struct, hash_value_t s2Struct)
{
    char *s1 = (char *)s1Struct.ptr;
    char *s2 = (char *)s2Struct.ptr;
    static __m128i zeroVector = _mm_setzero_si128();
    static __m128i cmpZero    = _mm_setzero_si128();

    do{
        __m128i *s1Vector = (__m128i *)s1;
        __m128i *s2Vector = (__m128i *)s2;
        __m128i result = _mm_cmpeq_epi8(*(__m128i *)s1, *(__m128i *)s2);
        int cmp = _mm_test_all_ones(result);
        if(!cmp) return 1;
        s1 += 16;
        s2 += 16;

        cmpZero = _mm_cmpeq_epi8(*s1Vector, zeroVector); // если хотя-бы один равен 0 соотв бит будет 0xFF
    }while(_mm_testz_si128(cmpZero, cmpZero));           //если хотя бы один бит равен 0xFF то  _mm_testz_si128(cmpZero, cmpZero) даст 0 

    return 0;
}


int32_t hasing_compare_string(hash_value_t s1, hash_value_t s2)
{
    return strcmp((char *)s1.ptr, (char *)s2.ptr);
}