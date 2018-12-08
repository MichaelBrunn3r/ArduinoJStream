#pragma once
#include "WString.h"

typedef uint8_t boolean;
typedef uint8_t byte;

class HardwareSerial {
    public:
        size_t print(const String &str);
        size_t print(const char str[]);
        size_t print(char c);
};

extern HardwareSerial Serial;
extern void delay(unsigned long);