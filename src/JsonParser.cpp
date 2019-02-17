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

        if(!stream->available()) return false;

        char c = stream->peek();
        return c != '}' && c != ']';
    }

    bool JsonParser::nextVal(size_t n) {
        return next(n);
    }

    bool JsonParser::nextKey(String* buf, size_t n) {
        if(!next(n)) return false;

        skipWhitespace();

        if(stream->peek() != '"') return false;
        stream->read();

        // Read and save key
        if(buf == NULL) exitString();
        else readString(*buf);

        skipWhitespace();

        if(stream->peek() != ':') return false;
        stream->read();
        
        skipWhitespace();

        return true;
    }

    bool JsonParser::findKey(const char* thekey) {
        NEXT_KEY:
        do {
            // Verify start of key 
            skipWhitespace();
            if(stream->peek() != '"') continue;  // not start of a string -> cannot be a key -> try matching next key
            stream->read();

            // Match key and thekey by comparing each char
            const char* thekey_it = thekey; // iterator over thekey
            while(stream->available() && *thekey_it != 0) {
                char c = stream->peek();

                // Escape char
                bool escaped = false;
                if(c == '\\') { // Escape char
                    stream->read();
                    c = JStream::escape(stream->peek());
                    
                    // Skip current key if char is unescapable
                    if(c==0) {
                        // stream->read(); <- Unneccesary, since '"' is escapeable
                        exitString();
                        goto NEXT_KEY;
                    } 

                    escaped = true;
                } else if(c == '"') { // Unescaped '"' terminates the key
                    stream->read(); // Exit key
                    goto NEXT_KEY; // try matching next key
                } 

                // Match key[idx] with thekey[idx]
                if(c != *thekey_it) { // key[idx] != thekey[idx] -> key doesn't match thekey
                    if(escaped) stream->read(); // Skip escaped char, in case it's a '"'
                    exitString();
                    goto NEXT_KEY;
                }

                // Advance to next char
                thekey_it++;
                stream->read();
            }

            // Check if matching was sucessful
            if(!(*thekey_it == 0 && stream->peek() == '"')) { // len(thekey) =/= len(key) -> one is prefix of the other -> no match
                exitString();
                continue; // try matching next key
            } else stream->read(); // Read closing '"'

            skipWhitespace(); // Whitespace between key and ':'

            if(stream->peek() != ':') continue; // No ':' after matched string -> not a valid json key -> try matching next key
            else stream->read();

            skipWhitespace(); // Whitespace before value (e.g "'key':   123")

            return true;
        } while(next());
        return false;
    } 

    bool JsonParser::find(Path& path) {
        if(!path.isValid) return false;

        for(auto it=path.begin(); it!=path.end(); ++it) {
            if(it!=path.begin()) {
                char c = stream->peek();
                if(c != '{' && c != '[') return false;
                stream->read();
            }
            
            if(it->type == PathSegmentType::OFFSET) {
                if(!next(it->val.offset)) return false;
            } else {
                if(!findKey(it->val.key)) return false;
            }
        }
        return true;
    }
    
    bool JsonParser::exit(size_t levels) {
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

    template<typename T>
    bool JsonParser::parseIntArray(std::vector<T>& vec) {
        skipWhitespace();

        if(stream->peek() != '[') return false;
        stream->read();

        while(stream->available() && !(JStream::isDecDigit(stream->peek()) || stream->peek() == '-')) stream->read();

        T num = 0;
        T sign = 1;

        while(stream->available()) {
            switch(stream->peek()) {
                case ',':
                    stream->read();

                    vec.push_back(num*sign); // Save read number
                    num = 0; // Reset num
                    sign = 1;

                    break;
                case ']':
                    stream->read();
                    vec.push_back(num*sign);
                    return true;
                case '-':
                    stream->read();
                    sign = -1;
                    break;
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + stream->read() - '0';
                    break;
                default:
                    stream->read();
            }
        }

        return false;
    }

    template bool JsonParser::parseIntArray<char>(std::vector<char>& vec);
    template bool JsonParser::parseIntArray<long>(std::vector<long>& vec);

    template<typename T>
    bool JsonParser::parseUIntArray(std::vector<T>& vec) {
        skipWhitespace();

        if(stream->peek() != '[') return false;
        stream->read();

        T num = 0;

        while(stream->available()) {
            switch(stream->peek()) {
                case ',':
                    stream->read();

                    vec.push_back(num); // Save read number
                    num = 0; // Reset num

                    break;
                case ']':
                    stream->read();
                    vec.push_back(num);
                    return true;
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + stream->read() - '0';
                    break;
                default:
                    stream->read();
            }
        }

        return false;
    }

    template bool JsonParser::parseUIntArray<byte>(std::vector<byte>& vec);
    template bool JsonParser::parseUIntArray<unsigned long>(std::vector<unsigned long>& vec);

    /////////////
    // Private //
    /////////////

    bool JsonParser::next(size_t n) {
        if(n == 0) return true;
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
                    if(nesting == 0) return false; // End of current object/array, no next key/value

                    // End of a nested object
                    stream->read();
                    nesting--;
                    break;
                case '"':
                    // Skip String
                    stream->read(); // Enter String
                    exitString(); // Exit String
                    break;
                case ',':
                    stream->read();
                    if(nesting != 0) break;
                        
                    // Reached start of next key/value
                    n--;
                    if(n == 0) {
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
            if(!isWhitespace(stream->peek())) break;
            stream->read();
        }
    }

    void JsonParser::exitString() {
        while(stream->available()) {
            char c = stream->read();
            if(c == '\\') stream->read();
            else if(c == '"') break;
        }
    }

    void JsonParser::readString(String& buf) {
        while(stream->available()) {
            char c = stream->read();
            if(c == '\\') {
                if(!stream->available()) break;
                c = JStream::escape(stream->read());
                if(c == 0) break;
            } else if (c == '"') break;

            buf += c;
        }
    }
}