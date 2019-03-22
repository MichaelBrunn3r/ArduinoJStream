#pragma once

#include <Stream.h>
#include <Arduino.h>
#include <WString.h>

class MockStringStream : public Stream {
    public:
        MockStringStream(const char* str, bool fakeNetworkHickups = false);

        int available();
        int read();
        int peek();
        size_t readBytes(char* buf, const int length);
        size_t readBytes(byte* buf, const int length);
        String peekString();
        size_t write(uint8_t);

    private:
        const char* str;

        bool fakeNetworkHickups;
        size_t chars_to_delay = 0;
};