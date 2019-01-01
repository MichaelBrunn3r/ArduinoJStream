#include "JsonValues/JsonNumber.h"

namespace Json {

    JsonNumber::JsonNumber(const char* num) : JsonValue(JsonValue::Type::NUM) {
        val = num; 
    }

    JsonNumber::JsonNumber(String num) : JsonValue(JsonValue::Type::NUM) {
        val = num;
    }

    const char* JsonNumber::toJsonString() {
        return val.c_str();
    }

}