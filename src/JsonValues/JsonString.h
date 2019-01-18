#pragma once

#include <JsonValues/JsonValue.h>
#include <WString.h>

namespace JStream {

    class JsonString : public JsonValue {
        public:
            JsonString(const char* str);
            JsonString(String str);
            const char* toJsonString() override;

        private:
            const char* val;
    };

}