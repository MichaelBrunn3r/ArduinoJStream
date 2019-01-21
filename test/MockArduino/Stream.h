#ifndef MOCK_Stream_HEADER
#define MOCK_Stream_HEADER

#include <Arduino.h>

class Stream {
    public:
        virtual int available() = 0;
        virtual int read() = 0;
        virtual size_t readBytes(char* buf, const int length) = 0;
        virtual size_t readBytes(byte* buf, const int length) = 0;
        virtual int peek() = 0;
};

#endif //MOCK_Stream_HEADER