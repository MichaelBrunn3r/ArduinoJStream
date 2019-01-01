#include "JsonObject.h"

namespace Json {

    const char* JsonObject::toString() {
        String buf = "{";
        for(std::map<const char*, JsonValue*>::iterator it=pairs.begin(); it!=pairs.end(); it++) {
            if(it != pairs.begin()) buf += ',';
            buf += it->first;
            buf += it->second->toString();
        }
        buf += "}";
        return buf.c_str();
    }

}