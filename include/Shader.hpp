#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    GLuint program;

    // Constructor: Reads and compiles shaders from files
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    // Use the shader program
    void use() { glUseProgram(program); }

    // Get uniform location
    GLint getUniform(const std::string& name) { return glGetUniformLocation(program, name.c_str()); }

    // Destructor: Cleanup shader program
    ~Shader() { glDeleteProgram(program); }

    void setMatrix4(const std::string& name, const glm::mat4& mat);
};

#endif // SHADER_HPP
