#version 410

// Input vertex position (in normalized device coordinates)
layout (location = 0) in vec3 position;

// Output UV coordinates for fragment shader
smooth out vec2 uv;

void main() {
    // Pass position directly to gl_Position (already in NDC)
    gl_Position = vec4(position, 1.0);
    
    // Calculate UV coordinates from position
    // Map from [-1,1] range to [0,1] range
    uv = 0.5f * (position.xy + vec2(1.0f));
}