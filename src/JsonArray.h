#pragma once

#include <JsonValue.h>

namespace Json {

    class JsonArray : public JsonValue {
        public:
            JsonArray();
            const char* toString() override;

        private:
            std::vector<JsonValue*> vals;
    };

}