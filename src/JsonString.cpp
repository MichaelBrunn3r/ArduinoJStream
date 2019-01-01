#include "JsonString.h"

namespace Json {

    JsonString::JsonString(const char* str) : JsonValue(JsonValue::Type::STR) {
        val = String(str);
    }

    JsonString::JsonString(String str) : JsonValue(JsonValue::Type::STR) {
        val = str;
    }

    const char* JsonString::toJsonString() {
        return String("\"" + val + "\"").c_str();
    }

}