#pragma once
#include <WString.h>
#include <Streams/InputStream.h>

namespace JStream {
    class StringStream : public InputStream {
        public:
            StringStream(String str);
            StringStream(const char* str, size_t length);
            ~StringStream();
            inline bool hasNext() {return idx < length;}
            inline char next() {return c_str[idx++];}
            inline char peek() {return c_str[idx];}
            void setTimeout(unsigned long t) {}
        
            private:
                char* c_str;
                unsigned int length = 0;
                unsigned int idx = 0;
    };
}