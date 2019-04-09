#include "WString.h"
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <cmath>

String::String() : std::string() {}
String::String(std::string str) : std::string(str) {}
String::String(const String& str) : std::string(str) {}
String::String(const char* cstr) : std::string(cstr) {}
String::String(char c) : std::string() {
    *this += c;
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
    *this += buf;
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
    *this += buf;
}

String::String(float val, unsigned char decimalPlaces) {
    std::ostringstream stream;
    stream << std::setprecision(decimalPlaces);
    stream << val;
    this->append(stream.str());
}

String::String(double val, unsigned char decimalPlaces) {
    std::ostringstream stream;
    stream << std::setprecision(decimalPlaces);
    stream << val;
    this->append(stream.str());
}

unsigned int String::length() { return size(); }

char String::charAt(unsigned int index) {return at(index);}

unsigned char String::concat(char c) {
    push_back(c);
}

unsigned char String::concat(const char* cstr, unsigned int length) {
    *this += cstr;
}

unsigned char String::concat(int num) {
    *this += num;
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

///////////////
// Operators //
///////////////

String& String::operator = (const String& str) {
    clear();
    append(str);
    return *this;
}

String& String::operator= (String&& str) {
    clear();
    append(str);
    return *this;
}

String& String::operator += (std::string str) {
    append(str);
    return *this;
}

String& String::operator += (const char* cstr) {
    append(cstr);
    return *this;
}

String& String::operator += (char c) {
    push_back(c);
    return *this;
}

String& String::operator += (unsigned char num) {
    *this += std::to_string(num);
    return *this;
}

String& String::operator += (int num) {
    *this += std::to_string(num);
    return *this;
}

String& String::operator += (unsigned int num) {
    *this += std::to_string(num);
    return *this;
}

String& String::operator += (long num) {
    *this += std::to_string(num);
    return *this;
}

String& String::operator += (unsigned long num) {
    *this += std::to_string(num);
    return *this;
}

String& String::operator += (float num) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << num;
    this->append(stream.str());
    return *this;
}

String& String::operator += (double num) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << num;
    this->append(stream.str());
    return *this;
}