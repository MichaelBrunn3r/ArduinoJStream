#pragma once
#include <Stream.h>
#include <Streams/InputStream.h>

class CharStream: public InputStream {
    public:
        CharStream(Stream* stream);
        ~CharStream();
        bool hasNext();
        char next();
        char peek();
        void setTimeout(unsigned long t) {timeout = t;}
    private:
        Stream* _stream;
        unsigned long timeout = 1000;
};