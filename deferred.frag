#version 410

// Inputs from vertex shader
in vec3 FragPos;      // World position
in vec3 Normal;       // Normal vector
in vec2 TexCoord;     // Texture coordinates

// Material properties
uniform bool hasTexture = false;
uniform sampler2D textureSampler;
uniform vec4 objectColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 baseColor = vec3(0.7f, 0.7f, 0.7f);

// Multiple render targets for G-Buffer
layout (location = 0) out vec3 gDiffuse;   // Diffuse color
layout (location = 1) out vec3 gNormal;    // Normal vectors
layout (location = 2) out vec3 gPosition;  // World positions

void main() {
    // Calculate diffuse color
    vec3 diffuseColor;
    if (hasTexture) {
        diffuseColor = texture(textureSampler, TexCoord).rgb * objectColor.rgb;
    } else {
        // Visualize the normals by mapping them to color space (optional)
        vec3 normNormal = normalize(Normal);
        vec3 normalColor = abs(normNormal * 0.5 + 0.5);
        
        // Mix base color with normal visualization
        diffuseColor = mix(baseColor, normalColor, 0.0) * objectColor.rgb;
    }
    
    // Output to G-Buffer
    gDiffuse = diffuseColor;
    gNormal = normalize(Normal);
    gPosition = FragPos;
}