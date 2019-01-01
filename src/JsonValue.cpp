#include "JsonValue.h"

namespace Json {

    const char* JsonValue::toString() {
        switch(type) {
            case Type::KW_TRUE: return "true";
            case Type::KW_FALSE: return "false";
            case Type::KW_NULL: return "null";
        }
    }
    
}