#pragma once

#include <JsonValue.h>

namespace Json {

    class JsonNumber : public JsonValue {
        public:
            JsonNumber(const char* num);
            JsonNumber(String num);
            const char* toString();

        private:
            String val;
    };

}