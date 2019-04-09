#pragma once

#include <Stream.h>
#include <Arduino.h>
#include <WString.h>

class MockStringStream : public Stream {
    public:
        MockStringStream();
        MockStringStream(String str);
        MockStringStream(const char* cstr);

        void setString(String str);

        int available();
        int read();
        int peek();
        size_t readBytes(char* buf, const int length);
        size_t readBytes(byte* buf, const int length);
        String peekString();
        size_t write(uint8_t);

        MockStringStream& operator= (const MockStringStream& other); 

    private:
        String mStr;
        const char* mCstr;
};