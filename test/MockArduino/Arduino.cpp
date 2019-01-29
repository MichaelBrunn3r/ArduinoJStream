#include "Arduino.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>

size_t HardwareSerial::print(const String &str) {std::cout << str;}
size_t HardwareSerial::print(const char* str) {std::cout << str;}
size_t HardwareSerial::print(char c) {std::cout << c;}

void delay(unsigned long t_ms) {
    usleep(t_ms * 1000);
}

static bool s_randomSeedCalled = false;

long random(long howbig) {
    if(howbig == 0) {
        return 0;
    }
    // if randomSeed was called, fall back to software PRNG
    if(!s_randomSeedCalled) randomSeed(time(0));
    uint32_t val = rand();
    return val % howbig;
}

long random(long howsmall, long howbig) {
    if(howsmall >= howbig) {
        return howsmall;
    }
    long diff = howbig - howsmall;
    return random(diff) + howsmall;
}

void randomSeed(unsigned long seed) {
    if(seed != 0) {
        srand(seed);
        s_randomSeedCalled = true;
    }
}

HardwareSerial Serial;