#ifndef TYPEREGISTRY_HPP
#define TYPEREGISTRY_HPP

#include <string>
#include <unordered_map>

class TypeRegistry {
private:
    static TypeRegistry* instance;
    std::unordered_map<std::string, int> typeMap;
    int nextTypeId;

    TypeRegistry() : nextTypeId(10) {}

public:
    static TypeRegistry* getInstance() {
        if (!instance) {
            instance = new TypeRegistry();
        }
        return instance;
    }

    int registerType(const std::string& typeName) {
        auto it = typeMap.find(typeName);
        if (it != typeMap.end()) {
            return it->second;
        }
        
        int typeId = nextTypeId++;
        typeMap[typeName] = typeId;
        return typeId;
    }

    int getNumTypes() const {
        return nextTypeId;
    }
};

// Define static member variable
inline TypeRegistry* TypeRegistry::instance = nullptr;

#endif // TYPEREGISTRY_HPP