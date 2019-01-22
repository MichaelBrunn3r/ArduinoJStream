#include "JsonParser.h"
#include <JsonUtils.h>
#include <cstring>

namespace JStream {
    void JsonParser::parseIntArray(std::vector<int>* vec, const char* json, size_t length) {
        const char* end = json+length;
        #define hasNext() (json < end)
        #define peek() *json
        #define next() *json++

        if(!hasNext() || peek() != '[') return;
        else next();

        int sign;
        int num;
        bool firstNumber = true;
        while(hasNext()) {
            if(peek() == ',') {
                next();
            } else if(peek() == ']') {
                next();
                return;
            } else if(!firstNumber) return;

            sign = 1;
            num = 0;
            if(hasNext() && peek() == '-') {
                next();
                sign = -1;
            }

            while(hasNext() && JStream::isDecDigit(peek())) {
                num = num*10 + next() - '0';
            }
            vec->push_back(sign*num);
        }

        #undef hasNext
        #undef peek
        #undef next
    }

    inline bool hasNext(Stream* stream) {
        if(!stream->available()) {
            stream->peek();
        }
        return stream->available() > 0;
    }

    void JsonParser::parseIntArray(std::vector<int>* vec, Stream* stream) {
        if(stream->peek() != '[') return;
        else stream->read();

        #define buf_capacity 32
        char buf[buf_capacity];
        size_t buf_len = buf_capacity;
        char* buf_it = buf;
        char* buf_end = buf;

        #define hasNext() (buf_it<buf_end)
        #define next() (*buf_it++)
        #define peek() (*buf_it)

        long num = 0;
        int sign = 1;

        while(true) {   
            if(buf_it == buf_end) {
                if(buf_len < buf_capacity) {
                    vec->push_back(num*sign);
                    break;
                }

                buf_len = stream->readBytesUntil(']', (char*) buf, buf_capacity);

                if(buf_len == 0) {
                    vec->push_back(num*sign);
                    break;
                }

                buf_it = buf;
                buf_end = buf+buf_len;

                if(hasNext() && peek() == '-') {
                    sign = -1;
                    next();
                }
            }
            
            if(hasNext() && peek() == ',') {
                next();
                vec->push_back(num*sign);
                
                num = 0;

                if(hasNext() && peek() == '-') {
                    sign = -1;
                    next();
                } else {
                    sign = 1;
                }
            }

            while(hasNext() && JStream::isDecDigit(peek())) {
                num = num*10 + next() - '0';
            }
        }
        
        #undef buf_capacity
        #undef hasNext
        #undef next
        #undef peek
    }
}