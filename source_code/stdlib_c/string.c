#include "../system_headers/string.h" // TODO: BIG PROBLEM -> I have two string.h files. Merge them

// TODO: I have two implementations of strlen because I forgot that I've already implemented it once
uint32_t strlen(const char *str)
{
    const char *s;
    for (s = str; *s; ++s) {} // Increment `s` until the null terminator is found
    return (s - str); // Return the difference, which is the length of the string
}

int strncmp(const char *str1, const char *str2, uint32_t n)
{
    while (n && *str1 && (*str1 == *str2)) {
        ++str1;
        ++str2;
        --n;
    }
    if (n == 0) {
        return 0; // The first n characters of both strings are equal
    } else {
        return (unsigned char)(*str1) - (unsigned char)(*str2);
    }
}
