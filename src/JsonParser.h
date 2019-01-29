#pragma once

#include <Arduino.h>
#include <vector>
#include <Stream.h>

namespace JStream {
    class JsonParser {
        public:
            static void parseIntArray(std::vector<int>* vec, const char* json, size_t length);
            static void parseIntArray(std::vector<int>* vec, Stream* stream);
            static const char* skipUntilKey(const char* json, const char* key);
            static void skipUntilKey(Stream* stream, const char* key);
    };
}