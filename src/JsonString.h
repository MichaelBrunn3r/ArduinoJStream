#pragma once

#include <JsonValue.h>

namespace Json {

    class JsonString : public JsonValue {
        public:
            JsonString(const char* str);
            JsonString(String str);
            const char* toString();

        private:
            String val;
    };

}