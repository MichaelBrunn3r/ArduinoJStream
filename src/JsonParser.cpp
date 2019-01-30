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

                if(peek() == '-') {
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

    void JsonParser::skipString(Stream* stream, bool insideString) {
        if(!insideString) {
            while(stream->available()) {
                if(stream->read() == '"') break;
            }
        }

        bool escaped = false;
        while(stream->available()) {
            char c = stream->read();
            if(c == '\\') escaped = true;
            else if(!escaped && c == '"') break;
            else escaped = false;
        }
    }

    const char* JsonParser::findKey(const char* json, const char* key) {
        #define hasNext() (*json)
        #define peek() (*json)
        #define next() (*json++)

        if(!*json || !*key) return json;
        
        while(hasNext()) {
            if(next() == '"') {
                // Match key
                const char* key_idx = key;
                while(hasNext()) {
                    if(peek() == *key_idx) {
                        key_idx++;
                        next();
                        if(!*key_idx) { // Reached string terminator -> matched key
                            if(!*key_idx && next() == '"' && peek() == ':') {
                                next();
                                goto EXIT_LOOP;
                            } 
                        } 
                    } else break;
                }

                // Couldn't match key. Skip until the end of the current key
                bool escaped = false;
                while(hasNext()) {
                    char c = next();
                    if(c == '\\') escaped = true;
                    else if(!escaped && c == '"') break;
                    else escaped = false;
                }
            }
        }
        EXIT_LOOP:
        return json;

        #undef hasNext
        #undef peek
        #undef next
    }

    void JsonParser::findKey(Stream* stream, const char* key) {

        if(!*key) return;
        
        while(stream->available()) {
            if(stream->read() == '"') {
                // Match key
                const char* key_idx = key;
                while(stream->available()) {
                    if(stream->peek() == *key_idx) {
                        key_idx++;
                        stream->read();
                        if(!*key_idx) { // Reached string terminator -> matched key
                            if(!*key_idx && stream->read() == '"' && stream->peek() == ':') {
                                stream->read();
                                goto EXIT_LOOP;
                            } 
                        } 
                    } else break;
                }

                // Couldn't match key. Skip until the current key
                skipString(stream, true);
            }
        }
        EXIT_LOOP:
        return;
    }

    void JsonParser::nextEntry(Stream* stream) {
        size_t nesting = 0;

        while(stream->available()) {
            switch(stream->peek()) {
                case '[':
                case '{':
                    stream->read();
                    nesting++;
                    break;
                case ']':
                case '}':
                    if(nesting == 0) return;
                    stream->read();
                    nesting--;
                    break;
                case '"':
                    skipString(stream, false);
                    break;
                case ',':
                    stream->read();
                    if(nesting == 0) return;
                    break;
                default:
                    stream->read();
            }
        }
    }

    void JsonParser::exitCollections(Stream* stream, size_t count) {
        if(count<1) return;

        size_t nesting = count-1;
        while(stream->available()) {
            switch(stream->read()) {
                case '[':
                case '{':
                    nesting++;
                    break;
                case ']':
                case '}':
                    if(nesting == 0) return;
                    nesting--;
                    break;
                case '"':
                    skipString(stream, true);
                    break;
            }
        }
    }
}