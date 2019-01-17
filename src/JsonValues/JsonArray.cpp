#include "JsonValues/JsonArray.h"

namespace JStream {

    JsonArray::JsonArray() : JsonValue(JsonValue::Type::ARR) {}

    const char* JsonArray::toJsonString() {
        String buf = "[";
        for(std::vector<JsonValue*>::iterator it=vals.begin(); it!=vals.end(); it++) {
            if(it != vals.begin()) buf += ',';
            buf += (*it)->toJsonString();
        }
        buf += ']';
    }

}