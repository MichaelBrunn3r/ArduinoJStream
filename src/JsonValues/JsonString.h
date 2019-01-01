#pragma once

#include <JsonValues/JsonValue.h>

namespace Json {

    class JsonString : public JsonValue {
        public:
            JsonString(const char* str);
            JsonString(String str);
            const char* toJsonString() override;

        private:
            String val;
    };

}