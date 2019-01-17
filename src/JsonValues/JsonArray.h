#pragma once

#include <JsonValues/JsonValue.h>

namespace JStream {

    class JsonArray : public JsonValue {
        public:
            JsonArray();
            const char* toJsonString() override;

        private:
            std::vector<JsonValue*> vals;
    };

}