#include "Shape.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

// Original constructor for backward compatibility
Shape::Shape(const size_t triangleCount, const std::vector<float>& vertexData) {
    if (triangleCount == 0 || vertexData.empty()) {
        std::cerr << "Error: Empty mesh data!" << std::endl;
        return;
    }

    size_t totalVertices = triangleCount * 3;

    if (vertexData.size() != totalVertices * 6) {
        std::cerr << "Error: Incorrect vertex data size!" << std::endl;
        return;
    }

    // Extract position and normal data into pos and norm vectors
    for (size_t i = 0; i < totalVertices; i++) {
        float x = vertexData[i * 6 + 0];
        float y = vertexData[i * 6 + 1];
        float z = vertexData[i * 6 + 2];
        pos.emplace_back(x, y, z);

        float nx = vertexData[i * 6 + 3];
        float ny = vertexData[i * 6 + 4];
        float nz = vertexData[i * 6 + 5];
        norm.emplace_back(nx, ny, nz);
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1); // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(totalVertices * 3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "Shape successfully created with " << triangleCount << " triangles and " << totalVertices << " vertices." << std::endl;
}

// New constructor for mesh with armature data
Shape::Shape(const size_t vertexCount, 
           const std::vector<float>& positionData,
           const std::vector<float>& normalData,
           const std::vector<float>& uvData,
           const std::vector<Bone>& bones,
           const std::vector<VertexBoneData>& vertexBoneData,
           bool hasBones) {
    
    if (vertexCount == 0 || positionData.empty() || normalData.empty()) {
        std::cerr << "Error: Empty mesh data!" << std::endl;
        return;
    }
    
    // Store bone data if present
    this->hasBones = hasBones;
    if (hasBones) {
        this->bones = bones;
        this->vertexBoneData = vertexBoneData;
        this->boneMatrices.resize(bones.size(), glm::mat4(1.0f));
    }
    
    // Extract position, normal, and UV data
    for (size_t i = 0; i < vertexCount; i++) {
        float x = positionData[i * 3 + 0];
        float y = positionData[i * 3 + 1];
        float z = positionData[i * 3 + 2];
        pos.emplace_back(x, y, z);
        
        float nx = normalData[i * 3 + 0];
        float ny = normalData[i * 3 + 1];
        float nz = normalData[i * 3 + 2];
        norm.emplace_back(nx, ny, nz);
        
        if (!uvData.empty() && uvData.size() >= vertexCount * 2) {
            float u = uvData[i * 2 + 0];
            float v = uvData[i * 2 + 1];
            uv.emplace_back(u, v);
        }
    }
    
    // Create and setup OpenGL buffers
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    
    // Calculate buffer sizes and offsets
    size_t positionSize = vertexCount * 3 * sizeof(float);
    size_t normalSize = vertexCount * 3 * sizeof(float);
    size_t uvSize = !uvData.empty() ? vertexCount * 2 * sizeof(float) : 0;
    
    // Calculate total buffer size
    size_t totalSize = positionSize + normalSize + uvSize;
    
    // Add space for bone data if needed
    size_t boneIndicesSize = 0;
    size_t boneWeightsSize = 0;
    std::vector<float> boneIndicesData;
    std::vector<float> boneWeightsData;
    
    if (hasBones) {
        boneIndicesSize = vertexCount * 4 * sizeof(float);
        boneWeightsSize = vertexCount * 4 * sizeof(float);
        totalSize += boneIndicesSize + boneWeightsSize;
        
        // Prepare bone data for GPU
        boneIndicesData.resize(vertexCount * 4);
        boneWeightsData.resize(vertexCount * 4);
        
        for (size_t i = 0; i < vertexCount; ++i) {
            for (int j = 0; j < 4; ++j) {
                boneIndicesData[i * 4 + j] = static_cast<float>(vertexBoneData[i].indices[j]);
                boneWeightsData[i * 4 + j] = vertexBoneData[i].weights[j];
            }
        }
    }
    
    // Allocate buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);
    
    // Upload position data
    size_t offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, positionSize, positionData.data());
    offset += positionSize;
    
    // Upload normal data
    glBufferSubData(GL_ARRAY_BUFFER, offset, normalSize, normalData.data());
    offset += normalSize;
    
    // Upload UV data if present
    if (!uvData.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, offset, uvSize, uvData.data());
        offset += uvSize;
    }
    
    // Upload bone data if present
    if (hasBones) {
        glBufferSubData(GL_ARRAY_BUFFER, offset, boneIndicesSize, boneIndicesData.data());
        offset += boneIndicesSize;
        
        glBufferSubData(GL_ARRAY_BUFFER, offset, boneWeightsSize, boneWeightsData.data());
    }
    
    // Set up vertex attributes
    offset = 0;
    glEnableVertexAttribArray(0);  // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    offset += positionSize;
    
    glEnableVertexAttribArray(1);  // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    offset += normalSize;
    
    // Set up UV attribute if present
    if (!uvData.empty()) {
        glEnableVertexAttribArray(2);  // UV
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)offset);
        offset += uvSize;
    }
    
    // Set up bone attributes if present
    if (hasBones) {
        glEnableVertexAttribArray(3);  // Bone indices
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)offset);
        offset += boneIndicesSize;
        
        glEnableVertexAttribArray(4);  // Bone weights
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    std::cout << "Shape successfully created with " << vertexCount << " vertices";
    if (hasBones) {
        std::cout << " and " << bones.size() << " bones";
    }
    std::cout << "." << std::endl;
}

Shape::~Shape() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    vao = 0;
    vbo = 0;
    std::cout << "Shape destroyed, OpenGL buffers deleted." << std::endl;
}

void Shape::updateBoneTransforms(const std::vector<glm::quat>& boneRotations) {
    if (!hasBones) return;
    
    // Check if we have the right number of rotations
    if (boneRotations.size() != bones.size()) {
        std::cerr << "Error: Bone rotation count doesn't match bone count!" << std::endl;
        return;
    }
    
    // Reset bone matrices to identity
    std::fill(boneMatrices.begin(), boneMatrices.end(), glm::mat4(1.0f));
    
    // Process bones in order (parents before children)
    for (size_t i = 0; i < bones.size(); ++i) {
        const Bone& bone = bones[i];
        glm::mat4& boneMatrix = boneMatrices[i];
        
        // Get bone rotation
        glm::mat4 rotationMatrix = glm::mat4_cast(boneRotations[i]);
        
        if (bone.parentIndex >= 0) {
            // Get parent transform
            const glm::mat4& parentMatrix = boneMatrices[bone.parentIndex];
            
            // Calculate parent displacement
            glm::vec3 parentDisplacement = glm::vec3(parentMatrix[3]);
            
            // Apply parent rotation to parent-to-child vector
            glm::vec3 rotatedVector = glm::mat3(parentMatrix) * bone.parentToChildVector;
            
            // Calculate current displacement
            glm::vec3 currentDisplacement = parentDisplacement + rotatedVector;
            
            // Build final matrix: parent rotation * this rotation
            boneMatrix = parentMatrix * rotationMatrix;
            
            // Set translation component
            boneMatrix[3] = glm::vec4(currentDisplacement - bone.localPosition, 1.0f);
        } else {
            // Root bone
            boneMatrix = rotationMatrix;
            boneMatrix[3] = glm::vec4(-bone.localPosition, 1.0f);
        }
    }
}

// Original load function for backward compatibility
bool loadMeshData(const std::string& filename, size_t& triangleCount, std::vector<float>& vertexData) {
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Error: Could not open mesh file " << filename << std::endl;
        return false;
    }

    std::string line;
    
    // Read number of triangles
    if (!std::getline(input, line)) {
        std::cerr << "Error: File format incorrect!" << std::endl;
        return false;
    }
    triangleCount = std::stoi(line);

    // Read vertex positions first
    for (size_t i = 0; i < triangleCount * 3; ++i) {
        float x, y, z;
        if (!(input >> x >> y >> z)) {
            std::cerr << "Error: Invalid vertex position data!" << std::endl;
            return false;
        }
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);
    }

    // Read normal vectors next
    for (size_t i = 0; i < triangleCount * 3; ++i) {
        float nx, ny, nz;
        if (!(input >> nx >> ny >> nz)) {
            std::cerr << "Error: Invalid normal data!" << std::endl;
            return false;
        }
        vertexData.push_back(nx);
        vertexData.push_back(ny);
        vertexData.push_back(nz);
    }

    input.close();
    std::cout << "Loaded " << triangleCount << " triangles from " << filename << std::endl;
    return true;
}

// New function to load mesh with armature data
bool loadMeshWithArmature(const std::string& filename, 
                         size_t& vertexCount, 
                         size_t& faceCount, 
                         std::vector<float>& positionData,
                         std::vector<float>& normalData, 
                         std::vector<float>& uvData,
                         std::vector<Bone>& bones,
                         std::vector<VertexBoneData>& vertexBoneData,
                         bool& hasBones) {
    
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Error: Could not open mesh file " << filename << std::endl;
        return false;
    }

    std::string line;
    hasBones = false;
    vertexCount = 0;
    faceCount = 0;
    
    // Parse header
    while (std::getline(input, line)) {
        // Skip comment lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        
        if (token == "vertices") {
            iss >> vertexCount;
        } else if (token == "faces") {
            iss >> faceCount;
        } else if (token == "bones") {
            size_t boneCount;
            iss >> boneCount;
            bones.resize(boneCount);
            hasBones = true;
        } else if (token == "bone") {
            int boneIndex;
            std::string boneName;
            int parentIndex;
            float x, y, z, px, py, pz;
            
            iss >> boneIndex;
            
            // Read bone name (it may contain spaces)
            char nameBuffer[256];
            iss.getline(nameBuffer, 256, '"');  // Skip until opening quote
            iss.getline(nameBuffer, 256, '"');  // Read until closing quote
            boneName = nameBuffer;
            
            iss >> parentIndex >> x >> y >> z >> px >> py >> pz;
            
            if (boneIndex >= 0 && static_cast<size_t>(boneIndex) < bones.size()) {
                bones[boneIndex].name = boneName;
                bones[boneIndex].parentIndex = parentIndex;
                bones[boneIndex].localPosition = glm::vec3(x, y, z);
                bones[boneIndex].parentToChildVector = glm::vec3(px, py, pz);
            }
        } else if (token == "v") {
            // Start of vertex data, break out to parse vertices
            break;
        }
    }
    
    // Prepare data containers
    positionData.resize(vertexCount * 3);
    normalData.resize(vertexCount * 3);
    uvData.resize(vertexCount * 2, 0.0f);  // Default UVs to 0
    
    if (hasBones) {
        vertexBoneData.resize(vertexCount);
    }
    
    // Process vertex data
    size_t vertexIndex = 0;
    input.seekg(0);  // Return to beginning of file
    
    // Skip header until we find vertex data
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        
        if (token == "v") {
            if (vertexIndex >= vertexCount) {
                std::cerr << "Error: Too many vertices in file" << std::endl;
                break;
            }
            
            // Position data
            float x, y, z, nx, ny, nz, u = 0.0f, v = 0.0f;
            iss >> x >> y >> z >> nx >> ny >> nz >> u >> v;
            
            // Store position
            positionData[vertexIndex * 3 + 0] = x;
            positionData[vertexIndex * 3 + 1] = y;
            positionData[vertexIndex * 3 + 2] = z;
            
            // Store normal
            normalData[vertexIndex * 3 + 0] = nx;
            normalData[vertexIndex * 3 + 1] = ny;
            normalData[vertexIndex * 3 + 2] = nz;
            
            // Store UV (placeholders, real UVs are per-face)
            uvData[vertexIndex * 2 + 0] = u;
            uvData[vertexIndex * 2 + 1] = v;
            
            // Parse bone data if present
            std::string boneToken;
            if (hasBones && (iss >> boneToken) && boneToken == "bones") {
                for (int i = 0; i < 4; ++i) {
                    int index;
                    float weight;
                    if (iss >> index >> weight) {
                        vertexBoneData[vertexIndex].indices[i] = index;
                        vertexBoneData[vertexIndex].weights[i] = weight;
                    }
                }
            }
            
            vertexIndex++;
        }
    }
    
    // Process face data for UVs (optional second pass)
    input.clear();
    input.seekg(0);  // Return to beginning of file
    
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        
        if (token == "f") {
            std::vector<int> faceIndices;
            int idx;
            
            // Read vertex indices
            while (iss >> idx && idx >= 0 && static_cast<size_t>(idx) < vertexCount) {
                faceIndices.push_back(idx);
            }
            
            // Reset stream and read past "f" and indices
            iss.clear();
            iss.str(line);
            iss >> token;  // skip "f"
            
            for (size_t i = 0; i < faceIndices.size(); ++i) {
                iss >> idx;  // skip index we already processed
            }
            
            // Check for UVs
            iss >> token;
            if (token == "uv") {
                for (size_t i = 0; i < faceIndices.size(); ++i) {
                    float u, v;
                    if (iss >> u >> v) {
                        // Store UV for this vertex index
                        int vertIndex = faceIndices[i];
                        if (vertIndex >= 0 && static_cast<size_t>(vertIndex) < vertexCount) {
                            uvData[vertIndex * 2 + 0] = u;
                            uvData[vertIndex * 2 + 1] = v;
                        }
                    }
                }
            }
        }
    }
    
    input.close();
    std::cout << "Loaded " << vertexCount << " vertices, " << faceCount << " faces";
    if (hasBones) {
        std::cout << ", and " << bones.size() << " bones";
    }
    std::cout << " from " << filename << std::endl;
    
    return true;
}

// Helper function to create a Shape from file
Shape* createShapeFromFile(const std::string& filename) {
    // Attempt to open the file to check format
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Error: Could not open mesh file " << filename << std::endl;
        return nullptr;
    }
    
    std::string line;
    if (!std::getline(input, line)) {
        std::cerr << "Error: File is empty" << std::endl;
        return nullptr;
    }
    
    input.close();
    
    // Try to detect file format
    if (line[0] == '#' || line.find("vertices") != std::string::npos) {
        // New format with armature support
        size_t vertexCount, faceCount;
        std::vector<float> positionData, normalData, uvData;
        std::vector<Bone> bones;
        std::vector<VertexBoneData> vertexBoneData;
        bool hasBones;
        
        if (!loadMeshWithArmature(filename, vertexCount, faceCount, positionData, 
                               normalData, uvData, bones, vertexBoneData, hasBones)) {
            return nullptr;
        }
        
        return new Shape(vertexCount, positionData, normalData, uvData, 
                       bones, vertexBoneData, hasBones);
    } else {
        // Original format
        size_t triangleCount;
        std::vector<float> vertexData;
        
        if (!loadMeshData(filename, triangleCount, vertexData)) {
            return nullptr;
        }
        
        return new Shape(triangleCount, vertexData);
    }
}