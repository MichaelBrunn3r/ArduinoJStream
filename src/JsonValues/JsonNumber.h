#pragma once

#include <JsonValues/JsonValue.h>

namespace JStream {

    class JsonNumber : public JsonValue {
        public:
            JsonNumber(const char* num);
            JsonNumber(String num);
            JsonNumber(int num);
            JsonNumber(long num);
            const char* toJsonString() override;

        private:
            const char* val;
    };

}