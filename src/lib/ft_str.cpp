#include "ft_str.hpp"

/**
 * Custom strndup function.
 * @param s => char
 * @param n => strlen.
 */
CHAR* Ft_str::Ft_Strndup(IN CHAR *s, IN SIZE_T n){
    if (!s) 
    return NULL;

    SIZE_T len = strnlen(s, n);
    CHAR *cpy = (CHAR *)malloc(sizeof(CHAR) * (len + 1));

    if (!cpy) 
        return NULL;

    memcpy(cpy, s, len);
    cpy[len] = '\0';

    return cpy;
}