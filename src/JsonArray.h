#pragma once

#include <JsonValue.h>

namespace Json {

    class JsonArray : public JsonValue {
        public:
            JsonArray();
            const char* toJsonString() override;

        private:
            std::vector<JsonValue*> vals;
    };

}