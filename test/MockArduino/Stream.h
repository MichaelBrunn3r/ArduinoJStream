#ifndef MOCK_Stream_HEADER
#define MOCK_Stream_HEADER

#include <Arduino.h>

#define NO_SKIP_CHAR 1 // a magic char not found in a valid ASCII numeric field

class Stream {
    public:
        Stream() : _timeout(1000) {}

        virtual int available() = 0;
        virtual int read() = 0;
        virtual int peek() = 0;
        virtual size_t readBytes(char* buf, const int length) = 0;
        virtual size_t readBytes(byte* buf, const int length) = 0;
        size_t readBytesUntil(char terminator, char *buffer, size_t length);
        virtual long parseInt(char skipChar = NO_SKIP_CHAR);
        virtual size_t write(uint8_t) = 0;

    protected:
        unsigned long _timeout;
        unsigned long _startMillis;
        int timedRead();
        int timedPeek();
        int peekNextDigit();
};

#endif //MOCK_Stream_HEADER