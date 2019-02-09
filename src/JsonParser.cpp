#include "JsonParser.h"
#include <JsonUtils.h>
#include <cstring>

namespace JStream {
    JsonParser::JsonParser() {}
    JsonParser::JsonParser(Stream* stream) : stream(stream) {}

    void JsonParser::parse(Stream* stream) {
        this->stream = stream;
    }

    bool JsonParser::atEnd() {
        skipWhitespace();

        if(stream->available()) {
            char c = stream->peek();
            return c != '}' && c != ']';
        } else return false;
    }

    bool JsonParser::nextVal() {
        return next();
    }

    String JsonParser::nextKey() {
        if(next()) {
            skipWhitespace();
            if(stream->peek() == '"') {
                stream->read();
                String str = readString();
                skipWhitespace();
                if(stream->peek() == ':') {
                    stream->read();
                    skipWhitespace();
                    return str;
                }
            }
        } 
        return "";
    }

    bool JsonParser::findKey(const char*& thekey) {
        while(stream->available()) {
            if(next()) {
                if(stream->peek() == '"') { // Enter string
                    stream->read();

                    // Match thekey with string
                    const char* thekey_idx = thekey;
                    while(stream->available() && *thekey) {
                        if(stream->peek() == *thekey_idx) { // string[idx] == key[idx] -> matched char
                            thekey_idx++;
                            stream->read();
                        } else break;
                    }

                    if(*thekey_idx == 0) { // Reached end of thekey -> matched thekey
                        if(stream->peek() == '"') { // String ended and matches thekey
                            stream->read();
                            skipWhitespace(); // Whitespace between string and ':' (e.g. "'key'  :123")
                            if(stream->peek() == ':') {
                                stream->read();
                                skipWhitespace(); // Whitespace before value (e.g "'key':   123")
                                return true;
                            } else {
                                /** The matched string was not followd by a ':', therefore it is not a valid json key.
                                 *  This error can only occur, when the key is malformed, or the stream is currently
                                 *  in an array.
                                 *  -> Skip to the next key-value pair
                                 */
                                continue;
                            }
                        } // string doesn't end here -> thekey is prefix of string -> no match
                    } 

                    exitString();
                }
            } else break;
        }
        return false;
    } 
    
    bool JsonParser::ascend(size_t levels) {
        if(levels == 0) return true;

        while(stream->available()) {
            switch(stream->peek()) {
                case '{':
                case '[':
                    // Start of a nested object
                    stream->read();
                    levels++;
                    break;
                case '}':
                case ']':
                    stream->read();
                    if(levels == 1) return true; // End of current object/array, no next key/value
                    else levels--; // End of a nested object
                    break;
                case '"':
                    // Skip String
                    stream->read(); // Enter String
                    exitString(); // Exit String
                    break;
                default:
                    stream->read();
            }
        }

        return false;
    }

    template<>
    void JsonParser::asArray<long>(std::vector<long>& vec) {
        skipWhitespace();
        if(stream->peek() != '[') return;
        else stream->read();

        #define buf_capacity 16
        char buf[buf_capacity];
        size_t buf_len = buf_capacity;
        char* buf_it = buf;
        char* buf_end = buf;

        #define hasNext() (buf_it<buf_end)
        #define read() (*buf_it++)
        #define peek() (*buf_it)

        long num = 0;
        int sign = 1;

        while(true) {   
            if(buf_it == buf_end) {
                if(buf_len < buf_capacity) {
                    vec.push_back(num*sign);
                    break;
                }

                buf_len = stream->readBytesUntil(']', (char*) buf, buf_capacity);

                if(buf_len == 0) {
                    vec.push_back(num*sign);
                    break;
                }

                buf_it = buf;
                buf_end = buf+buf_len;

                if(peek() == '-') {
                    sign = -1;
                    read();
                }
            }
            
            if(hasNext() && peek() == ',') {
                read();
                vec.push_back(num*sign);
                
                num = 0;

                if(hasNext() && peek() == '-') {
                    sign = -1;
                    read();
                } else {
                    sign = 1;
                }
            }

            while(hasNext() && JStream::isDecDigit(peek())) {
                num = num*10 + read() - '0';
            }
        }
        
        #undef buf_capacity
        #undef hasNext
        #undef next
        #undef peek
    }

    /////////////
    // Private //
    /////////////

    bool JsonParser::next() {
        size_t nesting = 0;

        while(stream->available()) {
            switch(stream->peek()) {
                case '{':
                case '[':
                    // Start of a nested object
                    stream->read();
                    nesting++;
                    break;
                case '}':
                case ']':
                    if(nesting == 0) {
                        // End of current object/array, no next key/value
                        return false;
                    } else {
                        // End of a nested object
                        stream->read();
                        nesting--;
                    }
                    break;
                case '"':
                    // Skip String
                    stream->read(); // Enter String
                    exitString(); // Exit String
                    break;
                case ',':
                    stream->read();
                    if(nesting == 0) {
                        // Reached start of next key/value
                        skipWhitespace();
                        return true;
                    }
                    break;
                default:
                    stream->read();
            }
        }

        // Stream ended, no next key/value
        return false;
    }

    void JsonParser::skipWhitespace() {
        while(stream->available()) {
            if(isWhitespace(stream->peek())) stream->read();
            else break;
        }
    }

    void JsonParser::exitString() {
        bool escaped = false;
        while(stream->available()) {
            char c = stream->read();
            if(c == '\\') escaped = true;
            else if(!escaped && c == '"') break;
            else escaped = false;
        }
    }

    String JsonParser::readString() {
        String str = "";
        bool escaped = false;
        while(stream->available()) {
            char c = stream->read();
            if(c == '\\') escaped = true;
            else if(!escaped && c == '"') break;
            else escaped = false;

            str += c;
        }
        return str;
    }
}