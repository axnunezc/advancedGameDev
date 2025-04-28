#version 410

// Input from vertex shader
smooth in vec3 normal;
smooth in vec2 texCoords;  // Receive texture coordinates
smooth in vec3 fragPos;    // Fragment position in world space

// Output
out vec4 color;

// Lighting parameters
uniform vec3 lightDirection = vec3(0.0f, -1.0f, -1.0f);
uniform vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
uniform vec3 ambientColor = vec3(0.2f, 0.2f, 0.2f);
uniform vec3 baseColor = vec3(0.7f, 0.7f, 0.7f);
uniform vec3 viewPos;  // Camera position for specular highlights

// Material properties
uniform bool hasTexture = false;
uniform sampler2D textureSampler;
uniform vec4 objectColor = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
    // Normalize the light direction and surface normal
    vec3 normLightDir = normalize(lightDirection);
    vec3 normNormal = normalize(normal);
    
    // Calculate diffuse lighting using Lambert model
    float lambert = clamp(-dot(normNormal, normLightDir), 0.0f, 1.0f);
    
    // Get base color - either from texture or uniform
    vec4 baseColorWithAlpha;
    if (hasTexture) {
        baseColorWithAlpha = texture(textureSampler, texCoords) * objectColor;
    } else {
        // Visualize the normals by mapping them to color space
        vec3 normalColor = abs(normNormal * 0.5 + 0.5);
        
        // Mix base color with normal visualization
        vec3 diffuseColor = mix(baseColor, normalColor, 0.5);
        baseColorWithAlpha = vec4(diffuseColor, 1.0) * objectColor;
    }
    
    // Specular lighting (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfwayDir = normalize(-normLightDir + viewDir);
    float spec = pow(max(dot(normNormal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * 0.3;  // Specular strength of 0.3
    
    // Calculate final color with ambient, diffuse, and specular components
    vec3 finalColor = ambientColor * baseColorWithAlpha.rgb + 
                     (baseColorWithAlpha.rgb * lambert + specular) * lightColor;
    
    // Output final color with alpha from base color
    color = vec4(finalColor, baseColorWithAlpha.a);
}