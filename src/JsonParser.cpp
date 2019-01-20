#include "JsonParser.h"
#include <JsonUtils.h>
#include <cstring>

namespace JStream {
    CharBuffer JsonParser::buf = CharBuffer(18);

    void JsonParser::parseIntArray(std::vector<int>* vec, const char* json, size_t length) {
        uint_fast16_t idx = 0;
        #define hasNext() (idx < length)
        #define peek() json[idx]
        #define next() json[idx++]

        if(!hasNext() || peek() != '[') return;
        else next();

        while(hasNext()) {
            if(peek() == ']') {
                next();
                return;
            }

            if(!vec->empty() && next() != ',') return;

            if(hasNext() && JStream::isNumDigit(peek())) {
                uint_fast16_t num_start = idx;
                while(hasNext() && JStream::isNumDigit(peek())) {
                    idx++;
                }
                char num[idx-num_start+1];
                memcpy(num, json+num_start, idx-num_start+1);
                vec->push_back(atoi(num));
            } else return;
        }

        #undef hasNext
        #undef peek
        #undef next
    }
}