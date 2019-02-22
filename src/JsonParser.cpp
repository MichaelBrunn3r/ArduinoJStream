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

    bool JsonParser::nextKey(String* buf) {
        skipWhitespace();

        do {
            if(stream->peek() != '"') continue; // not start of a string -> cannot be a key -> try matching next key
            stream->read();

            // Read and save key
            if(buf == NULL) {
                if(!skipString(true)) continue;
            } else {
                *buf = "";
                if(!readString(*buf, true)) continue;
            }

            skipWhitespace();

            if(stream->peek() != ':') continue;
            stream->read();

            skipWhitespace();
            return true;
        } while(next());
        
        return false;
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
    
    bool JsonParser::exitCollection(size_t levels) {
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

    bool JsonParser::skipCollection() {
        char c;
        do {
            c = stream->read();
            if(c == '[' || c == '{') return exitCollection();
        } while(c>0);

        return false;
    }

    bool JsonParser::skipString(bool inStr) {
        if(!inStr) if(stream->read() != '"') return false; // Read opening '"'

        char c = stream->read();
        do {
            if(c == '\\') stream->read();
            else if(c == '"') return true;

            c = stream->read();
        } while(c > 0);

        return false; // Stream ended without closing the string
    }

    bool JsonParser::readString(String& buf, bool inStr) {
        if(!inStr) if(stream->read() != '"') return false; // Read opening '"'

        char c = stream->read();
        while(c > 0) {
            if(c == '\\') {
                c = JStream::escape(stream->read());
                if(c == 0) break;
            } else if (c == '"') return true;

            buf += c;
            c = stream->read();
        }

        return false; // Stream ended without closing the string
    }

    bool JsonParser::parseInt(long& num) {
        num = 0;    
        long sign = 1;
        
        skipWhitespace();
        if(stream->peek() == '-') {
            stream->read();
            sign = -1;
        }

        bool moreThanOneDigit = false;
        char c;
        do {
            c = stream->peek();
            switch(c) {
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + stream->read() - '0';
                    moreThanOneDigit = true;
                    break;
                case '\r': case '\n': case '\t': case ' ':
                    stream->read();
                    break;
                default:
                    goto END_PARSING;
            }
        } while(c>0);
        END_PARSING:

        if(!moreThanOneDigit) return false;
        
        num *= sign;
        return true;
    }

    long JsonParser::parseInt() {
        long result;
        return parseInt(result) ? result : 0;
    }

    bool JsonParser::parseDecimal(double& num) {
        num = 0;
        double sign = 1;

        skipWhitespace();
        if(stream->peek() == '-') {
            stream->read();
            sign = -1;
        }
        
        // Parse Int
        bool moreThanOneDigit = false; 
        char c;
        while(c = stream->peek()) {
            switch(c) {
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + stream->read() - '0';
                    moreThanOneDigit = true;
                    break;
                case '\r': case '\n': case '\t': case ' ':
                    stream->read();
                    break;
                default:
                    goto END_PARSING_INT;
            }
        }
        END_PARSING_INT:

        if(!moreThanOneDigit) return false;

        // Parse Decimals
        if(c == '.') {
            stream->read();

            moreThanOneDigit = false;
            double fraction = 0.1;
            while(c = stream->peek()) {
                switch(c) {
                    case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                        num += (stream->read() - '0') * fraction;
                        fraction *= 0.1;
                        moreThanOneDigit = true;
                        break;
                    case '\r': case '\n': case '\t': case ' ':
                        stream->read();
                        break;
                    default:
                        goto END_PARSING_DECIMAL;
                }
            }
            END_PARSING_DECIMAL:

            if(!moreThanOneDigit) return false;
        }
        
        num *= sign;
        return true;
    }

    double JsonParser::parseDecimal() {
        double result;
        return parseDecimal(result) ? result : 0.0;
    }

    template<typename T>
    bool JsonParser::parseIntArray(std::vector<T>& vec, bool inArray) {
        if(!inArray) {
            skipWhitespace();

            if(stream->peek() != '[') return false;
            stream->read();
        }

        T num = 0;
        T sign = 1;

        size_t digits = 0;
        char c;
        do {
            c = stream->read();
            switch(c) {
                case ',':
                    if(digits > 0) vec.push_back(num*sign); // Save read number
                    num = 0; // Reset num
                    sign = 1;
                    digits = 0;

                    break;
                case ']':
                    if(digits > 0) vec.push_back(num*sign);
                    return true;
                case '-':
                    if(digits == 0) sign = -1;
                    break;
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + c - '0';
                    digits++;
                    break;
            }
        } while(c>0);

        return false;
    }

    template bool JsonParser::parseIntArray<byte>(std::vector<byte>& vec, bool inArray);
    template bool JsonParser::parseIntArray<char>(std::vector<char>& vec, bool inArray);
    template bool JsonParser::parseIntArray<long>(std::vector<long>& vec, bool inArray);

    /////////////
    // Private //
    /////////////

    bool JsonParser::next(size_t n) {
        if(n == 0) {
            skipWhitespace();
            return true;
        }

        size_t nesting = 0;

        char c;
        while((c = stream->read()) > 0) {
            switch(c) {
                case '{': case '[':
                    // Start of a nested object
                    exitCollection();

                    break;
                case '}': case ']':
                    return false; // End of current object/array, no next key/value
                case '"':
                    // Skip String
                    skipString(true); // Exit String
                    break;
                case ',':
                    if(nesting > 0) break;
                        
                    // Reached start of next key/value
                    n--;
                    if(n == 0) {
                        skipWhitespace();
                        return true;
                    }
                    
                    break;
            }
        }

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
}