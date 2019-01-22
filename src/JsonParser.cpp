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

    bool hasNext(Stream* stream) {
        if(!stream->available()) {
            stream->peek();
        }
        return stream->available() > 0;
    }

    void JsonParser::parseIntArray(std::vector<int>* vec, Stream* stream) {
        if(stream->peek() != '[') return;
        else stream->read();

        bool firstNumber = true;
        while(hasNext(stream)) {
            if(stream->peek() == ',') stream->read();
            else if(stream->peek() == ']') {
                stream->read();
                return;
            } else if(!firstNumber) return;

            char c = stream->peek();
            if(JStream::isDecDigit(c) || c == '-') {
                long num = stream->parseInt();
                if(!(c != '0' && num == 0)) {
                    vec->push_back(num);
                } else return;
            } else return;
        }
    }
}