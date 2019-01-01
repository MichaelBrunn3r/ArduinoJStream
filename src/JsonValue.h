#pragma once

#include <WString.h>
#include <map>
#include <vector>

namespace Json {
    class JsonValue {
        public:
            enum class Type : uint8_t {OBJ, ARR, NUM, STR, KW_TRUE, KW_FALSE, KW_NULL};

            JsonValue(Type t);
            virtual const char* toJsonString() {}

            Type type;
            static const JsonValue* KW_TRUE; 
            static const JsonValue* KW_FALSE; 
            static const JsonValue* KW_NULL; 
    };

}