#version 410

// Screen-space quad inputs
smooth in vec2 uv;

// G-Buffer texture samplers
uniform sampler2D diffuseTexture;   // Diffuse color texture
uniform sampler2D normalTexture;    // Normal vectors texture
uniform sampler2D positionTexture;  // World positions texture

// Light properties
// Maximum number of lights
#define MAX_LIGHTS 16

// Light arrays
uniform vec3 lightPositions[MAX_LIGHTS];  // Position of each light source
uniform vec3 lightColors[MAX_LIGHTS];     // Color of each light source
uniform int numActiveLights;              // Number of active lights (0 to MAX_LIGHTS)

// Light attenuation factors
uniform float constantFactor = 0.1;       // Constant attenuation
uniform float linearFactor = 0.01;        // Linear attenuation (beta)
uniform float quadraticFactor = 0.001;    // Quadratic attenuation (gamma)

// Other lighting parameters
uniform vec3 ambientColor = vec3(0.1, 0.1, 0.1);  // Ambient light color
uniform vec3 viewPos;                            // Camera position for specular

// Output to default framebuffer
out vec4 color;

void main() {
    // Sample data from G-Buffer textures
    vec3 diffuse = texture(diffuseTexture, uv).rgb;
    vec3 normal = normalize(texture(normalTexture, uv).rgb);
    vec3 position = texture(positionTexture, uv).rgb;
    
    // Skip lighting calculation for background (if normal is very small)
    if (length(normal) < 0.1) {
        color = vec4(diffuse, 1.0);
        return;
    }
    
    // Initialize lighting contribution with ambient
    vec3 lighting = ambientColor * diffuse;
    
    // Loop through all active lights
    for (int i = 0; i < numActiveLights; i++) {
        // Calculate vector from pixel to light
        vec3 lightDir = lightPositions[i] - position;
        float distance = length(lightDir);
        lightDir = normalize(lightDir);
        
        // Calculate Lambertian coefficient (L)
        float lambertian = max(dot(normal, lightDir), 0.0);
        
        // Calculate attenuation (alpha)
        float attenuation = 1.0 / (
            constantFactor + 
            linearFactor * distance +
            quadraticFactor * distance * distance
        );
        
        // Calculate specular component
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - position);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        vec3 specular = specularStrength * spec * lightColors[i];
        
        // Hadamard product (component-wise multiplication) of light color and diffuse color
        vec3 lightContribution = (lambertian * diffuse + specular) * lightColors[i] * attenuation;
        
        // Add this light's contribution to total lighting
        lighting += lightContribution;
    }
    
    // Output final color
    color = vec4(lighting, 1.0);
}