#include "JsonValues/JsonValue.h"

namespace Json {

    const JsonValue* JsonValue::KW_TRUE = new JsonValue(JsonValue::Type::KW_TRUE);
    const JsonValue* JsonValue::KW_FALSE = new JsonValue(JsonValue::Type::KW_FALSE);
    const JsonValue* JsonValue::KW_NULL = new JsonValue(JsonValue::Type::KW_NULL);

    JsonValue::JsonValue(JsonValue::Type t) : type(t) {}

    const char* JsonValue::toJsonString() {
        switch(type) {
            case Type::KW_TRUE: return "true";
            case Type::KW_FALSE: return "false";
            case Type::KW_NULL: return "null";
        }
    }
    
}