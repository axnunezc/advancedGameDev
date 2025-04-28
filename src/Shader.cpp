    #include "Shader.hpp"
    #include <sys/stat.h>

    // Function to check if file exists
    bool fileExists(const std::string& filename) {
        struct stat buffer;
        return (stat(filename.c_str(), &buffer) == 0);
    }

    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
        // Check if files exist
        if (!fileExists(vertexPath) || !fileExists(fragmentPath)) {
            std::cerr << "Error: Shader file not found! Check that " << vertexPath << " and " << fragmentPath << " exist." << std::endl;
            return;
        }

        // Read Vertex Shader
        std::ifstream vShaderFile(vertexPath);
        std::stringstream vShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        std::string vShaderStr = vShaderStream.str();
        const char* vShaderCode = vShaderStr.c_str();

        // Read Fragment Shader
        std::ifstream fShaderFile(fragmentPath);
        std::stringstream fShaderStream;
        fShaderStream << fShaderFile.rdbuf();
        std::string fShaderStr = fShaderStream.str();
        const char* fShaderCode = fShaderStr.c_str();

        // Compile Vertex Shader
        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vShaderCode, NULL);
        glCompileShader(vert);
        int success;
        char infoLog[512];
        glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vert, 512, NULL, infoLog);
            std::cerr << "Vertex Shader Compilation Failed\n" << infoLog << std::endl;
        }

        // Compile Fragment Shader
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fShaderCode, NULL);
        glCompileShader(frag);
        glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(frag, 512, NULL, infoLog);
            std::cerr << "Fragment Shader Compilation Failed\n" << infoLog << std::endl;
        }

        // Link Shaders into a Program
        program = glCreateProgram();
        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Shader Program Linking Failed\n" << infoLog << std::endl;
        }

        // Cleanup
        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    void Shader::setMatrix4(const std::string& name, const glm::mat4& mat) {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
