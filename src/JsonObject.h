#pragma once

#include <JsonValue.h>

namespace Json {

    class JsonObject : public JsonValue {
        public:
            JsonObject();
            const char* toJsonString() override;
        
        private:
            std::map<const char*, JsonValue*> pairs;
    };
    
}