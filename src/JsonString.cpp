#include "JsonString.h"

namespace Json {

    const char* JsonString::toString() {
        return val.c_str();
    }

}