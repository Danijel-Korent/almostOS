#include "../system_headers/string.h" // TODO: BIG PROBLEM -> I have two string.h files. Merge them

// TODO: I have two implementations of strlen because I forgot that I've already implemented it once
uint32_t strlen(const char *string)
{
    const char *itr;

    // Find null-terminator
    for (itr = string; *itr != 0; itr++);

    // The difference between this two pointers is the length of the string
    return (itr - string); 
}

int strncmp(const char *str1, const char *str2, uint32_t n)
{
    // TODO: WTF is this shit? Did I maybe use AI te get this?
    //       I don't remember using AI, but I would also not write code like this, I hope
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
