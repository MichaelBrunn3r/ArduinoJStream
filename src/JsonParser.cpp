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
        if(buf == NULL) skipString(true);
        else readString(*buf);

        skipWhitespace();

        if(stream->peek() != ':') return false;
        stream->read();
        
        skipWhitespace();

        return true;
    }

    bool JsonParser::findKey(const char* thekey) {
        NEXT_KEY:
        skipWhitespace();
        do {
            // Verify start of key 
            if(stream->peek() != '"') continue;  // not start of a string -> cannot be a key -> try matching next key
            stream->read();

            // Match key and thekey by comparing each char
            const char* thekey_it = thekey; // iterator over thekey
            while(*thekey_it) {
                char c = stream->read();

                if(c <= 0) return false; // Stream ended

                if(c == '\\') { // Escape char
                    c = JStream::escape(stream->read());
                    
                    // Skip current key if char is unescapable
                    if(c==0) {
                        skipString(true);
                        goto NEXT_KEY;
                    }
                } else if(c == '"') goto NEXT_KEY; // Unescaped '"' terminates key -> thekey is longer than key -> try matching next key

                // Match key[idx] with thekey[idx]
                if(c != *thekey_it) { // key[idx] != thekey[idx] -> key doesn't match thekey
                    skipString(true);
                    goto NEXT_KEY;
                }

                // Advance to next char
                thekey_it++;
            }

            // Check if matching was sucessful
            if(*thekey_it != 0 || stream->peek() != '"') { // len(thekey) =/= len(key) -> one is prefix of the other -> no match
                skipString(true);
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

    bool JsonParser::find(const char* path) {
        const char* start = path;
        while(*path) { 
            if(path != start) {
                char c = stream->peek();
                if(c != '{' && c != '[') return false;
                stream->read();
            }

            if(*path == '[') { // array path segment
               path++;

                // Read offset
                size_t offset = 0;
                while(*path) {
                    if(JStream::isDecDigit(*path)) {
                        offset = offset*10 + *path++ - '0';
                    } else if(*path == ']') {
                        path++;
                        break;
                    }
                }

                if(!next(offset)) return false;

                // offset (i.e. '[...]') can only be followed by another offset or the start of a key (i.e. '/') 
                if(*path && *path != '/' && *path != '[') return false;
            } else { // key path segment
                if(*path == '/') path++;

                // Read key
                String keyBuf = "";
                while(*path) {
                    switch(*path) {
                        case '[':
                            goto END_READ_KEY;
                        case '/': 
                            path++;
                            goto END_READ_KEY;
                        case '\\': 
                            path++;
                            if(*path != '[' && *path != '/') keyBuf += '\\';
                        default: 
                            keyBuf += *path++;
                    }
                }    
                END_READ_KEY:

                if(!findKey(keyBuf.c_str())) return false;
            }
        }
        return true;
    }
    
    bool JsonParser::exit(size_t levels) {
        if(levels == 0) return true;

        char c;
        do {
            c = stream->read();
            switch(c) {
                case '{': case '[':
                    levels++;
                    break;
                case '}': case ']':
                    if(levels == 1) return true;
                    levels--;
                    break;
                case '"':
                    skipString(true);
                    break;
            }
        } while(c>0);

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

        char c;
        do {
            c = stream->read();
            switch(c) {
                case ',':
                    vec.push_back(num*sign); // Save read number
                    num = 0; // Reset num
                    sign = 1;

                    break;
                case ']':
                    vec.push_back(num*sign);
                    return true;
                case '-':
                    sign = -1;
                    break;
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + c - '0';
                    break;
            }
        } while(c>0);

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

        char c;
        do {
            c = stream->read();
            switch(c) {
                case ',':
                    vec.push_back(num); // Save read number
                    num = 0;

                    break;
                case ']':
                    vec.push_back(num); // Save read number
                    return true;
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + c - '0';
                    break;
            }
        } while(c>0);

        return false; // Stream ended without closing the array
    }

    template bool JsonParser::parseUIntArray<byte>(std::vector<byte>& vec);
    template bool JsonParser::parseUIntArray<unsigned long>(std::vector<unsigned long>& vec);

    /////////////
    // Private //
    /////////////

    bool JsonParser::next(size_t n) {
        if(n == 0) return true;
        size_t nesting = 0;

        char c;
        do {
            c = stream->peek();
            switch(c) {
                case '{': case '[':
                    // Start of a nested object
                    stream->read();
                    exit(1);

                    break;
                case '}': case ']':
                    return false; // End of current object/array, no next key/value
                case '"':
                    // Skip String
                    stream->read(); // Enter String
                    skipString(true); // Exit String
                    break;
                case ',':
                    stream->read();
                    if(nesting > 0) break;
                        
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
        } while(c>0);

        // Stream ended, no next key/value
        return false;
    }

    void JsonParser::skipWhitespace() {
        char c;
        do {
            c = stream->peek();
            if(isNotWhitespace(c)) break;
            stream->read();
        } while(c > 0);
    }

    void JsonParser::skipString(bool inStr) {
        if(!inStr) stream->read(); // Read opening '"'

        char c = stream->read();
        do {
            if(c == '\\') stream->read();
            else if(c == '"') break;

            c = stream->read();
        } while(c > 0);
    }

    void JsonParser::readString(String& buf) {
        char c = stream->read();
        while(c > 0) {
            if(c == '\\') {
                c = JStream::escape(stream->read());
                if(c == 0) break;
            } else if (c == '"') break;

            buf += c;
            c = stream->read();
        }
    }
}