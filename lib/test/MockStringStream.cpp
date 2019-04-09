#include "MockStringStream.h"
#include <cstring>

MockStringStream::MockStringStream() : MockStringStream("") {}

MockStringStream::MockStringStream(String str) {
    setString(str);
}

MockStringStream::MockStringStream(const char* cstr) {
    mStr = String(cstr);
    mCstr = mStr.c_str();
}

void MockStringStream::setString(String str) {
    mStr = str;
    mCstr = mStr.c_str();
}

int MockStringStream::available() {
    if(*mCstr == 0) return -1;
    long charsRead = mCstr - mStr.c_str(); 
    return mStr.length() - charsRead;
    // return *cstr;
}

int MockStringStream::read() {
    if(available() > 0) {
        return *mCstr++;
    } else return -1;
}

int MockStringStream::peek() {    
    return *mCstr;
}

size_t MockStringStream::readBytes(char* buf, const int length){
    int read = 0;
    for(int i=0; i<length && available() > 0; i++) {
        *buf++ = this->read();
        read++;
    }
    return read;
}

size_t MockStringStream::readBytes(byte* buf, const int length) {
    int read = 0;
    for(int i=0; i<length && available() > 0; i++) {
        *buf++ = this->read();
        read++;
    }
    return read;
}

String MockStringStream::peekString() {
    return String(mCstr);
}

size_t MockStringStream::write(uint8_t) {
    return 1;
}

MockStringStream& MockStringStream::operator= (const MockStringStream& other) {
    mStr = other.mStr;
    mCstr = mStr.c_str();
    return *this;
}