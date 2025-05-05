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

int32_t hasing_compare_string_simd(hash_value_t s1Struct, hash_value_t s2Struct) {
    const char *s1 = (const char *)s1Struct.ptr;
    const char *s2 = (const char *)s2Struct.ptr;
    
    const __m128i zero = _mm_setzero_si128();
    
    uintptr_t align_mask = ~(uintptr_t)0xF;
    const char *s1_aligned = (const char *)((uintptr_t)s1 & align_mask);
    const char *s2_aligned = (const char *)((uintptr_t)s2 & align_mask);
    
    if (s1 != s1_aligned || s2 != s2_aligned) {
        __m128i v1 = _mm_loadu_si128((const __m128i *)s1);
        __m128i v2 = _mm_loadu_si128((const __m128i *)s2);
        
        __m128i cmp = _mm_cmpeq_epi8(v1, v2);
        __m128i zero_cmp = _mm_cmpeq_epi8(v1, zero);
        
        unsigned mask = _mm_movemask_epi8(cmp);
        unsigned zero_mask = _mm_movemask_epi8(zero_cmp);
        
        if ((mask | zero_mask) != 0xFFFF) {
            return 1;
        }
        
        s1 += 16 - ((uintptr_t)s1 & 0xF);
        s2 += 16 - ((uintptr_t)s2 & 0xF);
    }
    
    while (1) {
        __m128i v1 = _mm_load_si128((const __m128i *)s1);
        __m128i v2 = _mm_load_si128((const __m128i *)s2);
        
        __m128i cmp = _mm_cmpeq_epi8(v1, v2);
        __m128i zero_cmp = _mm_cmpeq_epi8(v1, zero);
        
        unsigned mask = _mm_movemask_epi8(cmp);
        unsigned zero_mask = _mm_movemask_epi8(zero_cmp);
        
        if ((mask | zero_mask) != 0xFFFF) {
            return 1;
        }
        if (zero_mask != 0) {
            return 0;
        }
        
        s1 += 16;
        s2 += 16;
    }
}

int32_t hasing_compare_string(hash_value_t s1, hash_value_t s2)
{
    return strcmp((char *)s1.ptr, (char *)s2.ptr);
}