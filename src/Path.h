#pragma once

#include <Arduino.h>
#include <vector>

namespace JStream
{
    class PathSegment; // Forward declaration

    // Representation of a path that can be used to find Json elements
    class Path : public std::vector<PathSegment> {
        public:
            bool isValid;

            Path(const char* path_str);
            Path(const char* path_str, size_t n);

            /**
             * @brief Appends a path to the end of the path
             * 
             * Formatting:
             *  - "[n]": OFFSET segment, n-th child element in the parent json array/object
             *  - "akey": KEY segment, child "akey" of the parent json object
             *  - "key1/key2[2]/key3": KEY segments are seperated from other segments with a '/'
             *  - "akey[2][2]": OFFSET segments can be appended directly to other segments
             */
            bool append(const char* path_str);
    };

    enum class PathSegmentType : byte {OFFSET, KEY};
    struct PathSegment {
        PathSegment(size_t offset);
        PathSegment(const char* key);
        PathSegment(const char* key, size_t len);
        PathSegment(String& key);
        PathSegment(const PathSegment& other);
        ~PathSegment();

        PathSegmentType type;
        union {
            size_t offset;
            const char* key;
        } val;
    };
} // Jstream
