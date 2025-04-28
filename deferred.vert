#version 410

// Input vertex attributes
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 texCoord;     // UV coordinates
layout(location = 3) in vec4 boneIndices;  // Indices of bones affecting this vertex
layout(location = 4) in vec4 boneWeights;  // Weights of each bone's influence

// Uniform matrices
uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

// Animation uniforms
uniform bool hasArmature = false;
uniform int boneCount = 0;
uniform mat4 boneMatrices[100];  // Array of bone transformation matrices

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    vec4 positionTransformed;
    vec3 normalTransformed;
    
    // Check if this vertex has bone weights and armature is enabled
    if (hasArmature && (boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w > 0.0)) {
        // Initialize for skinning
        positionTransformed = vec4(0.0);
        normalTransformed = vec3(0.0);
        
        // Apply bone transformations
        for (int i = 0; i < 4; i++) {
            int boneIndex = int(boneIndices[i]);
            float weight = boneWeights[i];
            
            if (weight > 0.0 && boneIndex >= 0 && boneIndex < boneCount) {
                // Apply bone transformation weighted by its influence
                mat4 boneTransform = boneMatrices[boneIndex];
                positionTransformed += weight * (boneTransform * vec4(pos, 1.0));
                
                // Transform normal with bone matrix (ignoring translation)
                mat3 boneRotation = mat3(boneTransform);
                normalTransformed += weight * (boneRotation * norm);
            }
        }
        
        // Normalize the normal
        normalTransformed = normalize(normalTransformed);
    } else {
        // No bone weights or armature, use original vertex data with model transform
        positionTransformed = model * vec4(pos, 1.0);
        normalTransformed = normalize(mat3(transpose(inverse(model))) * norm);
    }
    
    // Set output normal
    Normal = normalTransformed;
    
    // Output texture coordinates
    TexCoord = texCoord;
    
    // Set fragment position for lighting calculations
    FragPos = positionTransformed.xyz;
    
    // Final position transformation
    gl_Position = proj * view * positionTransformed;
}