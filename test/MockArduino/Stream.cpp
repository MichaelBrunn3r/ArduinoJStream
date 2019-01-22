#include "Stream.h"

long Stream::parseInt() {
    boolean isNegative = false;
    long value = 0;
    int c;

    c = peek();
    // ignore non numeric leading characters
    if(!(c >= 48 && c <= 57 || c == '-')) return 0; // zero returned if timeout

    do {
        if(c == '-')
            isNegative = true;
        else if(c >= '0' && c <= '9')        // is c a digit?
            value = value * 10 + c - '0';
        read();  // consume the character we got with peek
        c = peek();
    } while((c >= '0' && c <= '9'));

    if(isNegative) value = -value;
    return value;
}