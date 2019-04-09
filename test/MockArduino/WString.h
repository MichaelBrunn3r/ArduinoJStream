#pragma once
#include <string>
#include <stdint.h>
#include <iostream>

#define DEC 10
#define HEX 16
#define OCT 8

class String : public std::string {
    public:
        String();
        String(std::string str);
        String(const String& str);
        String(const char* cstr);
        
        explicit String(char c);
        explicit String(int val, unsigned char base=DEC);
        explicit String(long val, unsigned char base=DEC);
        explicit String(float val, unsigned char decimalPlaces = 2);
        explicit String(double val, unsigned char decimalPlaces = 2);

        unsigned int length(void);

        char charAt(unsigned int index);
        unsigned char concat(char c);
        unsigned char concat(int num);
        unsigned char concat(const char* cstr, unsigned int length);

        long toInt(void) const;

        int compareTo(const String &s) const;
        unsigned char equals(const String &s) const;
        unsigned char equals(const char *cstr) const;

        String& operator = (const String& str);
        String& operator= (String&& str);

        String& operator += (std::string str);
        String& operator += (const char* cstr);
        String& operator += (char c);
        String& operator += (unsigned char num);
        String& operator += (int num);
        String& operator += (unsigned int num);
        String& operator += (long num);
        String& operator += (unsigned long num);
        String& operator += (float num);
        String& operator += (double num);

        friend inline String operator+ (const std::string& lhs, int val) { return std::string(lhs) += val; }
        friend inline String operator+ (const std::string& lhs, const char cstr) { return std::string(lhs) += cstr; }

    protected:
        using std::string::size;
        using std::string::at;
        using std::string::append;
        using std::string::push_back;
};