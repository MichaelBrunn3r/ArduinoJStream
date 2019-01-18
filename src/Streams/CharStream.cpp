#include <Streams/CharStream.h>
#include <Arduino.h>

namespace JStream {
    CharStream::CharStream(Stream* stream) : _stream(stream) {}
    CharStream::~CharStream() {}

    bool CharStream::hasNext() {
        if(_stream->available()) return true;
        else {
            delay(timeout);
            return _stream->available();
        }
    }

    char CharStream::next() {
        if(hasNext()) return _stream->read();
        else return -1;
    }

    char CharStream::peek() {
        if(hasNext()) return _stream->peek();
        else return -1;
    }
}