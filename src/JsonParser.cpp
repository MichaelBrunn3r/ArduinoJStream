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
}