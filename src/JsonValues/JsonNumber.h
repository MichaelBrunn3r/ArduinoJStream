#pragma once

#include <JsonValues/JsonValue.h>

namespace Json {

    class JsonNumber : public JsonValue {
        public:
            JsonNumber(const char* num);
            JsonNumber(String num);
            const char* toJsonString() override;

        private:
            String val;
    };

}