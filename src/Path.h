#pragma once

#include <Arduino.h>
#include <vector>

namespace JStream
{
    class PathSegment; // Forward declaration

    class Path : public std::vector<PathSegment> {
        public:
            bool isValid;

            Path(const char* path_str, size_t n=3);

        private:
            /**
             * @brief Converts a string to a path that can be used to find Json elements
             * 
             * Parses and splits the path string into OFFSET ('[...]') and KEY ('akey') segments.
             * Segments followed by a KEY segment have to be seperated with '/' (e.g. "key1/key2", "key1[1]/key2"). 
             * OFFSET segments however are not seperated (e.g. "akey[1]", "akey[1][1]").
             * 
             */
            bool compile(const char* path_str);
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
