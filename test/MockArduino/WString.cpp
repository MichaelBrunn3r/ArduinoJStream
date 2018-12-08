#include "WString.h"
#include <string.h>

String::String(const char* str) : std::string(str) {}
String::String(const char c) : std::string() {
    push_back(c);
}

char String::charAt(unsigned int index) {return at(index);}

unsigned char String::concat(char c) {
    push_back(c);
}