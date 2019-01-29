#include "MockStringStream.h"

MockStringStream::MockStringStream(const char* str, bool fakeNetworkHickups) : str(str), fakeNetworkHickups(fakeNetworkHickups) {}

int MockStringStream::available() {
    return *str;
}

int MockStringStream::read() {
    if(chars_to_delay == 0 && fakeNetworkHickups) {
        delay(random(5,100));
        chars_to_delay = random(20,200);
    }

    if(available()) {
        if(fakeNetworkHickups) chars_to_delay--;
        return *str++;
    }
    else return -1;
}

int MockStringStream::peek() {    
    return *str;
}

size_t MockStringStream::readBytes(char* buf, const int length){
    int read = 0;
    for(int i=0; i<length && available(); i++) {
        *buf++ = this->read();
        read++;
    }
    return read;
}

size_t MockStringStream::readBytes(byte* buf, const int length) {
    int read = 0;
    for(int i=0; i<length && available(); i++) {
        *buf++ = this->read();
        read++;
    }
    return read;
}

size_t MockStringStream::write(uint8_t) {
    return 1;
}