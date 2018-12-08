#include "Arduino.h"
#include <iostream>
#include <unistd.h>

size_t HardwareSerial::print(const String &str) {std::cout << str;}
size_t HardwareSerial::print(const char str[]) {std::cout << str;}
size_t HardwareSerial::print(char c) {std::cout << c;}

void delay(unsigned long t_ms) {
    usleep(t_ms * 1000);
}

HardwareSerial Serial;