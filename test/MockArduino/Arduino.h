#ifndef MOCK_Arduino_HEADER
#define MOCK_Arduino_HEADER

#include "WString.h"
#include <ctime>

#define DEC 10
#define HEX 16
#define OCT 8

typedef uint8_t boolean;
typedef uint8_t byte;

class HardwareSerial {
    public:
        size_t print(const String &str);
        size_t print(const char* str);
        size_t print(char c);
};

extern HardwareSerial Serial;
extern void delay(unsigned long);
long random(long);
long random(long, long);
void randomSeed(unsigned long);

#endif //MOCK_Arduino_HEADER