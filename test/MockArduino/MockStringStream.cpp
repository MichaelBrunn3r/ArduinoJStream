#include "MockStringStream.h"

MockStringStream::MockStringStream(String str) : str(str) {}

int MockStringStream::available() {
    return idx+1 < str.length();
}

int MockStringStream::read() {
    if(available()) return str[idx++];
    else return 0;
}

size_t MockStringStream::readBytes(char* buf, const int length){
    for(int i=0; i<length && available(); i++) {
        *buf++ = (char) str[i];
        idx++;
    }
}

size_t MockStringStream::readBytes(byte* buf, const int length) {
    for(int i=0; i<length && available(); i++) {
        *buf++ = (byte) str[i];
        idx++;
    }
}

int MockStringStream::peek() {
    return str[idx];
}