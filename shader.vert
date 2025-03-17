#version 410
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

smooth out vec3 normal;

void main() {
    normal = normalize(model * vec4(norm, 0.0f)).xyz;
    gl_Position = proj * view * model * vec4(pos, 1.0f);
}
