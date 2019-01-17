#pragma once
#include <string>
#include <stdint.h>

#define DEC 10
#define HEX 16
#define OCT 8

class String : public std::string {
    public:
        String(const char* str);
        explicit String(const char c);
        String(int val, unsigned char base=DEC);
        String(long val, unsigned char base=DEC);

        char charAt(unsigned int index);
        unsigned char concat(char c);
};