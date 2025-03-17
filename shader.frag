#version 410
smooth in vec3 normal;
out vec4 color;
void main()
{
    vec3 lightDirection = normalize(vec3(0.0f, -1.0f, -1.0f));
    float lambert = clamp(-dot(normal, lightDirection), 0.0f, 1.0f);
    vec4 diffuse = vec4(lambert * abs(normal), 1.0f);
    color = diffuse;
}