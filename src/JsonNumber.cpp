#include "JsonNumber.h"

namespace Json {

    const char* JsonNumber::toJsonString() {
        return val.c_str();
    }

}