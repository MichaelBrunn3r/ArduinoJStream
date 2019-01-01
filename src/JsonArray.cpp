#include "JsonArray.h"

namespace Json {

    const char* JsonArray::toString() {
        String buf = "[";
        for(std::vector<JsonValue*>::iterator it=vals.begin(); it!=vals.end(); it++) {
            if(it != vals.begin()) buf += ',';
            buf += (*it)->toString();
        }
        buf += ']';
    }

}