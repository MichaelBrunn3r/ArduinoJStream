#pragma once

#include <JsonValues/JsonValue.h>
#include <Streams/InputStream.h>
#include <Streams/StringStream.h>
#include <vector>
#include <CharBuffer.h>

namespace JStream {
    class JsonParser {
        public:
            static CharBuffer buf;
            static void parseIntArray(std::vector<int>* vec, const char* json, size_t length);
    };
}