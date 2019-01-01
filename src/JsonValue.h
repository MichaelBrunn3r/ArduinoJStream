#pragma once

#include <WString.h>
#include <map>
#include <vector>

namespace Json {
    class JsonValue {
        public:
            enum class Type : uint8_t {OBJ, ARR, NUM, STR, KW_TRUE, KW_FALSE, KW_NULL};

            JsonValue(Type type);
            virtual const char* toString() {}

            Type type;
    };
}