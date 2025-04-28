#version 410

// Input UV coordinates from vertex shader
smooth in vec2 uv;

// Samplers for G-Buffer textures
uniform sampler2D diffuseTexture;  // Diffuse color
uniform sampler2D normalTexture;   // Normal vectors
uniform sampler2D positionTexture; // World positions

// Display mode
// 0 = Combined result (deferred lighting)
// 1 = Diffuse buffer only
// 2 = Normal buffer only
// 3 = Position buffer only
uniform int displayMode = 0;

// Output color to default framebuffer
out vec4 color;

void main() {
    // Sample data from G-Buffer textures
    vec3 diffuse = texture(diffuseTexture, uv).rgb;
    vec3 normal = texture(normalTexture, uv).rgb;
    vec3 position = texture(positionTexture, uv).rgb;
    
    // Depending on display mode, output different information
    if (displayMode == 1) {
        // Display diffuse colors
        color = vec4(diffuse, 1.0);
    }
    else if (displayMode == 2) {
        // Display normal vectors (map from [-1,1] to [0,1] for visualization)
        color = vec4(normal * 0.5 + 0.5, 1.0);
    }
    else if (displayMode == 3) {
        // Display positions (normalized for visualization)
        // We need to adjust this for better visualization since position values can be large
        // Here we're just taking the fractional part
        color = vec4(fract(position * 0.1), 1.0);
    }
    else {
        // Default mode: combined result (this would be calculated in a deferred lighting pass)
        // For now, just output diffuse color
        color = vec4(diffuse, 1.0);
    }
}