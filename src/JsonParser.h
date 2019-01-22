#pragma once

#include <Arduino.h>
#include <vector>
#include <Stream.h>

namespace JStream {
    class JsonParser {
        public:
            void parseIntArray(std::vector<int>* vec, const char* json, size_t length);
            void parseIntArray(std::vector<int>* vec, Stream* stream);
    };
}