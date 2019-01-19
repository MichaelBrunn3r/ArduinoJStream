#include "StringStream.h"
#include <string.h>

namespace JStream {
    StringStream::StringStream(String str) : length(str.length()) {
        c_str = (char*) malloc(sizeof(char) * length);
        strcpy(c_str, str.c_str());
    }
    StringStream::StringStream(const char* str, size_t length) : length(length) {
        c_str = (char*) malloc(sizeof(char) * length);
        strcpy(c_str, str);
    }   
    StringStream::~StringStream() {
        delete[] c_str;
    }
}