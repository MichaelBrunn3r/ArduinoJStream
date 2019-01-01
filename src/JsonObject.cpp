#include "JsonObject.h"

namespace Json {

    JsonObject::JsonObject() : JsonValue(JsonValue::Type::OBJ) {}

    const char* JsonObject::toJsonString() {
        String buf = "{";
        for(std::map<const char*, JsonValue*>::iterator it=pairs.begin(); it!=pairs.end(); it++) {
            if(it != pairs.begin()) buf += ',';
            buf += it->first;
            buf += it->second->toJsonString();
        }
        buf += "}";
        return buf.c_str();
    }

}