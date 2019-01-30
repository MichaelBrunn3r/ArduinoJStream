#pragma once

#include <Arduino.h>
#include <vector>
#include <Stream.h>

namespace JStream {
    class JsonParser {
        public:
            static void parseIntArray(std::vector<int>* vec, const char* json, size_t length);
            static void parseIntArray(std::vector<int>* vec, Stream* stream);
            /**
             * @brief Skips until after the next occuring String in a Stream
             * 
             * @param insideString Indicates if this method should skip over a String or exit the current one
             */
            static void skipString(Stream* stream, bool insideString = false);
            static const char* skipUntilKey(const char* json, const char* key);
            static void skipUntilKey(Stream* stream, const char* key);
            /** 
             * @brief Skips to the next key/value in the current object/array
             * 
             * WARNING: The current char can not be inside a String !!!
             * 
             * This method reads the stream until it discovers the next entry, 
             * which must be preceded by a ',' and is in the same nesting. It doesn't
             * exit the current object/array, therefore it will terminate at the
             * closing '}' or ']'.
             */
            static void nextEntry(Stream* stream);
            /**
             * @brief Exits the current object/array
             * 
             * WARNING: The current char can not be inside a String !!!
             */
            static inline void exitCollection(Stream* stream) {
                exitCollections(stream, 1);
            }
            /**
             * @brief Exits a specified number of overlying objects/arrays
             * 
             * WARNING: The current char can not be inside a String !!!
             * 
             * @param count The number of overlying objects/arrays to exit. Must be >0
             */
            static void exitCollections(Stream* stream, size_t count=1);
    };
}