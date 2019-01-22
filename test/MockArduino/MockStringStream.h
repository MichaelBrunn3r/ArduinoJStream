#pragma once

#include <Stream.h>
#include <Arduino.h>
#include <WString.h>

class MockStringStream : public Stream {
    public:
        MockStringStream(String str);

        int available();
        int read();
        size_t readBytes(char* buf, const int length);
        size_t readBytes(byte* buf, const int length);
        int peek();
    private:
        size_t idx = 0;
        String str;
};