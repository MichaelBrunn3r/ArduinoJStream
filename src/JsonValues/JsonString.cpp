#include "JsonValues/JsonString.h"

namespace Json {

    JsonString::JsonString(const char* str) : val(str), JsonValue(JsonValue::Type::STR) {}
    JsonString::JsonString(String str) : val(str.c_str()), JsonValue(JsonValue::Type::STR) {}

    const char* JsonString::toJsonString() {
        String buf = "\"";
        buf += val;
        buf += "\"";
        return buf.c_str();
    }

}