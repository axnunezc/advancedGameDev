#include "CollisionResponder.hpp"
#include <algorithm>
#include "TypeRegistry.hpp"

CollisionResponder::CollisionResponder() : numTypes(10) {
    resize();
}

int CollisionResponder::getIndex(int typeA, int typeB) {
    // Ensure typeA <= typeB to always access the same cell
    if (typeA > typeB) {
        std::swap(typeA, typeB);
    }
    
    // Add validation to avoid -1 return
    if (typeA < 0 || typeB < 0) {
        std::cerr << "Error: Negative type IDs: " << typeA << ", " << typeB << std::endl;
        return 0; // Return a safe default index
    }
    
    // Formula for triangular array
    return (typeB * (typeB + 1)) / 2 + typeA;
}

void CollisionResponder::resize() {
    // Get the maximum possible type ID
    int maxTypeId = std::max(2, TypeRegistry::getInstance()->getNumTypes());
    
    // Calculate the maximum possible index for the triangular array
    int maxIndex = (maxTypeId * (maxTypeId + 1)) / 2;
    
    // Resize the callback table if needed
    if (callbackTable.size() <= maxIndex) {
        callbackTable.resize(maxIndex + 1, nullptr);
    }
}

void CollisionResponder::registerCallback(int typeA, int typeB, CollisionCallback callback) {
    // Make sure the table is large enough
    resize();
    
    // Get the index for these types
    int index = getIndex(typeA, typeB);
    
    // Validate the index
    if (index >= 0 && index < callbackTable.size()) {
        callbackTable[index] = callback;
    } else {
        std::cerr << "Error: Invalid collision callback index: " << index 
                  << " (max: " << callbackTable.size() - 1 << ")" << std::endl;
        
        // Expand the table if needed
        if (index >= 0) {
            callbackTable.resize(index + 1, nullptr);
            callbackTable[index] = callback;
            std::cout << "Expanded callback table to handle index " << index << std::endl;
        }
    }
}

void CollisionResponder::processCollision(GameObject* objA, GameObject* objB) {
    int typeA = objA->getTypeId();
    int typeB = objB->getTypeId();
    
    int index = getIndex(typeA, typeB);
    
    // Check if we have a callback registered
    if (index >= 0 && index < callbackTable.size() && callbackTable[index]) {
        // Invoke the callback
        callbackTable[index](objA, objB);
    }
}