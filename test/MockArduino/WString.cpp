#include "WString.h"
#include <string.h>
#include <stdio.h>
#include <cstdlib>

String::String() : std::string("") {}
String::String(const char* str) : std::string(str) {}
String::String(const char c) : std::string() {
    push_back(c);
}

String::String(int val, unsigned char base) {
    size_t buf_len = 2 + 8 * sizeof(int); 
    char buf[buf_len];
    switch(base) {
        case DEC: 
            snprintf(buf, buf_len, "%d", val);
            break;
        case HEX:
            snprintf(buf, buf_len, "%X", val);
            break;
        case OCT:
            snprintf(buf, buf_len, "%o", val);
            break;
    }
    *this = buf;
}

String::String(long val, unsigned char base) {
    size_t buf_len = 2 + 8 * sizeof(long); 
    char buf[buf_len];
    switch(base) {
        case DEC: 
            snprintf(buf, buf_len, "%ld", val);
            break;
        case HEX:
            snprintf(buf, buf_len, "%lX", val);
            break;
        case OCT:
            snprintf(buf, buf_len, "%lo", val);
            break;
    }
    *this = buf;
}

char String::charAt(unsigned int index) {return at(index);}

unsigned char String::concat(char c) {
    push_back(c);
}

long String::toInt(void) const {
    return std::atoi(this->c_str());
}

int String::compareTo(const String &s) const {
    compare(s);
}

unsigned char String::equals(const String &s2) const {
    return compare(s2) == 0;
}

unsigned char String::equals(const char *cstr) const {
    return compare(cstr) == 0;
}