#pragma once
#include <string>

class String : public std::string {
    public:
        String(const char* str);
        explicit String(const char c);

        char charAt(unsigned int index);
        unsigned char concat(char c);
};