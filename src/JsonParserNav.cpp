#include "JsonParser.h"
#include <JsonUtils.h>
#include <cstring>
#include <Internals/NumAccumulator.h>

namespace JStream {
    bool JsonParser::nextVal(size_t n) {
        return next(n);
    }

    String JsonParser::nextKey() {
        String buf = "";

        skipWhitespace();
        do {
            if(stream->peek() != '"') continue; // not start of a string -> cannot be a key -> try matching next key
            stream->read();

            buf = "";
            if(!readString(buf, true)) continue;

            char c = skipWhitespace();

            if(c != ':') continue;
            stream->read();

            skipWhitespace();
            return buf;
        } while(next());
        
        return "";
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
                int c = stream->read();

                if(c == 0) return false; // Stream ended

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

            int c = skipWhitespace(); // Whitespace between key and ':'

            if(c != ':') continue; // No ':' after matched string -> not a valid json key -> try matching next key
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
                int c = stream->peek();
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
                int c = stream->peek();
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

    char JsonParser::enterCollection() {
        char c = skipWhitespace();
        if(c != '[' && c != '{') return 0;
        stream->read();
        return c;
    }
    
    bool JsonParser::exitCollection(size_t levels) {
        if(levels == 0) return true;

        int c;
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
        } while(c!=-1 && c!=0);

        return false;
    }

    bool JsonParser::skipCollection() {
        int c;
        do {
            c = stream->read();
            if(c == '[' || c == '{') return exitCollection();
        } while(c!=-1 && c!=0);

        return false;
    }

    bool JsonParser::skipString(bool inStr) {
        if(!inStr) {
            skipWhitespace();
            if(stream->read() != '"') return false; // Read opening '"'
        }

        int c = stream->read();
        do {
            if(c == '\\') stream->read();
            else if(c == '"') return true;

            c = stream->read();
        } while(c != 0 && c != -1);

        return false; // Stream ended without closing the string
    }

    /////////////
    // Private //
    /////////////

    bool JsonParser::next(size_t n) {
        if(n == 0) {
            skipWhitespace();
            return true;
        }

        size_t nesting = 0;

        int c = stream->read();
        while(c!=-1 && c!=0) {
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

            c = stream->read();
        }

        // Stream ended, no next key/value
        return false;
    }

    char JsonParser::skipWhitespace() {
        char c;
        do {
            c = stream->peek();
            if(isNotWhitespace(c)) break;
            stream->read();
        } while(c > 0);

        return c;
    }
}