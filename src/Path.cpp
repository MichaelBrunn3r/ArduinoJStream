#include "Path.h"
#include <Internals/JsonUtils.h>
#include <cstring>

namespace JStream {

    ////////////
    /// Path ///
    ////////////

    Path::Path(const char* path_str) {
        isValid = append(path_str);
    }

    Path::Path(const char* path_str, size_t n) {
        reserve(n);
        isValid = append(path_str);
    }

    bool Path::append(const char* path_str) {
        while(*path_str) { 
            if(*path_str == '[') { // array path segment
               path_str++;

                // Read offset
                size_t offset = 0;
                while(*path_str) {
                    if(Internals::isDecDigit(*path_str)) {
                        offset = offset*10 + *path_str++ - '0';
                    } else if(*path_str == ']') {
                        path_str++;
                        break;
                    }
                }

                push_back(PathSegment(offset));

                // offset (i.e. '[...]') can only be followed by another offset or the start of a key (i.e. '/') 
                if(*path_str && *path_str != '/' && *path_str != '[') return false;
            } else { // key path segment
                if(*path_str == '/') path_str++;

                // Read key
                String keyBuf = "";
                while(*path_str) {
                    switch(*path_str) {
                        case '[':
                            goto END_READ_KEY;
                        case '/': 
                            path_str++;
                            goto END_READ_KEY;
                        case '\\': 
                            path_str++;
                            if(*path_str != '[' && *path_str != '/') keyBuf += '\\';
                        default: 
                            keyBuf += *path_str++;
                    }
                }    
                END_READ_KEY:

                push_back(PathSegment(keyBuf.c_str(), keyBuf.length()));
            }
        }
        return true;
    }

    ///////////////////
    /// PathSegment ///
    ///////////////////

    PathSegment::PathSegment(size_t offset) : type(PathSegmentType::OFFSET) {
        val.offset = offset;
    } 
    PathSegment::PathSegment(const char* key) : PathSegment(key, std::strlen(key)) {}
    PathSegment::PathSegment(const char* key, size_t len) : type(PathSegmentType::KEY) {
        val.key = (char*) memcpy(new char[len+1], key, len+1);
    } 
    PathSegment::PathSegment(String& str) : PathSegment(str.c_str(), str.length()) {}
    PathSegment::PathSegment(const PathSegment& other) {
        type = other.type;
        if(type == PathSegmentType::KEY) {
            size_t len = std::strlen(other.val.key);
            val.key = (char*) memcpy(new char[len+1], other.val.key, len+1);
        } else {
            val.offset = other.val.offset;
        }
    }
    PathSegment::~PathSegment() {
        if(type == PathSegmentType::KEY) delete[] val.key;
    }

} // JStream
