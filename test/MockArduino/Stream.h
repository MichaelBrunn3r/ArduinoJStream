#pragma once

class Stream {
    public:
        virtual int available() = 0;
        virtual int read() = 0;
        virtual int peek() = 0;
};