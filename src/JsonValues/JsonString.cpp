#include "JsonValues/JsonString.h"

namespace Json {

    JsonString::JsonString(const char* str) : val(String(str)), JsonValue(JsonValue::Type::STR) {}
    JsonString::JsonString(String str) : val(str), JsonValue(JsonValue::Type::STR) {}

    const char* JsonString::toJsonString() {
        String buf = "\"";
        buf += val;
        buf += "\"";
        return buf.c_str();
    }

}