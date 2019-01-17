#include "JsonValues/JsonNumber.h"

namespace Json {

    JsonNumber::JsonNumber(const char* num) : val(num), JsonValue(JsonValue::Type::NUM) {}
    JsonNumber::JsonNumber(String num) : val(num.c_str()), JsonValue(JsonValue::Type::NUM) {}
    JsonNumber::JsonNumber(int num) : val(String(num).c_str()), JsonValue(JsonValue::Type::NUM) {}
    JsonNumber::JsonNumber(long num) : val(String(num).c_str()), JsonValue(JsonValue::Type::NUM) {}

    const char* JsonNumber::toJsonString() {
        return val;
    }
}