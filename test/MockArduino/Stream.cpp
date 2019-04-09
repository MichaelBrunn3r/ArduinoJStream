#include "Stream.h"
#include <ctime>



int Stream::timedRead() {
    int c;
    _startMillis = millis();
    do {
        c = read();
        if(c >= 0)
            return c;
        yield();
    } while(millis() - _startMillis < _timeout);
    return -1;     // -1 indicates timeout
}

int Stream::timedPeek() {
    int c;
    _startMillis = millis();
    do {
        c = peek();
        if(c >= 0)
            return c;
        yield();
    } while(millis() - _startMillis < _timeout);
    return -1;     // -1 indicates timeout
}

int Stream::peekNextDigit() {
    int c;
    while(1) {
        c = timedPeek();
        if(c < 0)
            return c;  // timeout
        if(c == '-')
            return c;
        if(c >= '0' && c <= '9')
            return c;
        read();  // discard non-numeric
    }
}

long Stream::parseInt(char skipChar) {
    boolean isNegative = false;
    long value = 0;
    int c;

    c = peekNextDigit();
    // ignore non numeric leading characters
    if(c < 0)
        return 0; // zero returned if timeout

    do {
        if(c == skipChar)
            ; // ignore this charactor
        else if(c == '-')
            isNegative = true;
        else if(c >= '0' && c <= '9')        // is c a digit?
            value = value * 10 + c - '0';
        read();  // consume the character we got with peek
        c = timedPeek();
    } while((c >= '0' && c <= '9') || c == skipChar);

    if(isNegative)
        value = -value;
    return value;
}

size_t Stream::readBytesUntil(char terminator, char *buffer, size_t length) {
    if(length < 1)
        return 0;
    size_t index = 0;
    while(index < length) {
        int c = timedRead();
        if(c < 0 || c == terminator)
            break;
        *buffer++ = (char) c;
        index++;
    }
    return index; // return number of characters, not including null terminator
}

String Stream::readString() {
    String ret;
    while(available() > 0) {
        ret += (char) read();
    }
    return ret;
}

size_t Stream::readBytes(char* buf, const int length) {
    size_t count = 0;
        while(count < length) {
            int c = timedRead();
            if(c < 0)
                break;
            *buf++ = (char) c;
            count++;
        }
    return count;
}