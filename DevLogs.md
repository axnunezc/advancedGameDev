# **Development Log & Engine Documentation**

## **📌 Project: Baseline Game Engine - Rendering a Cube**

### **Developer:** [Your Name]

### **Date:** [Current Date]

### **Goal:** Implement a basic rendering system to display a 3D object (cube) in OpenGL using SDL2.

---

## **📝 Development Logs**

### **🔹 Phase 1: OpenGL and SDL Initialization**

#### ✅ **Tasks Completed**

- Initialized SDL and OpenGL.
- Created an OpenGL context and verified `GLEW` initialization.
- Configured SDL with `SDL_GL_SetAttribute()` for OpenGL version, depth buffer, and double buffering.

#### 🐛 **Bugs & Fixes**

- **Bug:** `GLEW Initialization Failed` → **Fix:** Ensured OpenGL context is created before calling `glewInit()`.
- **Bug:** `SDL_GL_SwapWindow()` was swapping buffers before drawing → **Fix:** Moved swap call after rendering.

---

### **🔹 Phase 2: Shader Compilation and Linking**

#### ✅ **Tasks Completed**

- Implemented a `Shader` class to load, compile, and link GLSL shaders.
- Verified shader compilation success using `glGetShaderiv()`.
- Implemented a simple vertex and fragment shader.

#### 🐛 **Bugs & Fixes**

- **Bug:** `Shader Program Linking Failed: Input of fragment shader 'normal' not written by vertex shader`\
  **Fix:** Ensured the vertex shader properly outputs `normal` to be used by the fragment shader.

---

### **🔹 Phase 3: Loading and Storing Mesh Data**

#### ✅ **Tasks Completed**

- Implemented `loadMeshData()` to read vertex positions and normals from `mesh.cse`.
- Verified correct parsing of mesh data using debug print statements.
- Stored loaded data in `std::vector<float>` and passed it to OpenGL.

#### 🐛 **Bugs & Fixes**

- **Bug:** Mesh data was not printing correctly from `pos` → **Fix:** Printed `pos` after loading to verify correct storage.
- **Bug:** `Error: Expected 216 values, but read 200!` → **Fix:** Ensured the file contained all required vertex and normal data.

---

### **🔹 Phase 4: Creating VAO/VBO for Rendering**

#### ✅ **Tasks Completed**

- Created a `Shape` class to manage VAO and VBO.
- Uploaded vertex data to OpenGL using `glBufferData()`.
- Configured `glVertexAttribPointer()` to correctly handle positions and normals.

#### 🐛 **Bugs & Fixes**

- **Bug:** `glDrawArrays()` caused an OpenGL error (`1282`) → **Fix:** Ensured `glBindVertexArray()` was called before rendering.
- **Bug:** No objects appeared on screen → **Fix:** Verified camera position and projection matrix.

---

### **🔹 Phase 5: Rendering the Mesh**

#### ✅ **Tasks Completed**

- Rendered the mesh using `glDrawArrays(GL_TRIANGLES, 0, myMesh.getVertexCount());`
- Implemented `glPolygonMode(GL_LINE)` to debug the wireframe.
- Adjusted camera position to ensure the object was in view.

#### 🐛 **Bugs & Fixes**

- **Bug:** Screen was pink, nothing rendering → **Fix:** Removed duplicate `glClear()` calls.
- **Bug:** Object was too small → **Fix:** Adjusted projection and camera settings.
- **Bug:** `VAO = 0, VBO = 0` before rendering → **Fix:** Ensured correct VAO/VBO binding.

---

## **📖 Engine Documentation**

### **📌 Overview**

The **Baseline Game Engine** is a lightweight engine that initializes OpenGL, loads 3D mesh data, and renders objects using GLSL shaders. It is built using **SDL2 and OpenGL 4.1**.

---

### **🔹 Core Components**

| **Module**       | **Description**                                             |
| ---------------- | ----------------------------------------------------------- |
| `SDL_Manager`    | Manages SDL window creation and updates.                    |
| `Shader`         | Loads, compiles, and links GLSL shaders.                    |
| `Shape`          | Stores vertex data, creates VAO/VBO, and handles rendering. |
| `loadMeshData()` | Reads and parses mesh data from `.cse` files.               |

---

### **🔹 SDL Manager (Window Management)**

#### **Class:** `SDL_Manager`

Handles SDL initialization and OpenGL context creation.

```cpp
void SDL_Manager::updateWindows() {
    if (count > 0) {
        SDL_GL_SwapWindow(windows[0]); // Ensure OpenGL window updates correctly
    }
    for (size_t i = 1; i < count; ++i) {
        SDL_UpdateWindowSurface(windows[i]);
    }
}
```

---

### **🔹 Shader Management**

#### **Class:** `Shader`

Handles GLSL shader loading and compilation.

```cpp
GLuint Shader::getUniform(const std::string& name) {
    return glGetUniformLocation(program, name.c_str());
}
```

---

### **🔹 Shape Management**

#### **Class:** `Shape`

Manages VAO/VBO and renders mesh data.

```cpp
void Shape::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, pos.size());
    glBindVertexArray(0);
}
```

---

### **🔹 Mesh Loading**

#### **Function:** `loadMeshData(filename, triangleCount, vertexData)`

Reads a `.cse` file and loads vertex positions and normals.

```cpp
bool loadMeshData(const std::string& filename, size_t& triangleCount, std::vector<float>& vertexData) {
    std::ifstream input(filename);
    if (!input.is_open()) return false;

    std::string line;
    std::getline(input, line);
    triangleCount = std::stoul(line);

    for (size_t i = 0; i < triangleCount * 3; ++i) {
        float x, y, z;
        input >> x >> y >> z;
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);
    }

    return true;
}
```

---

## **🎯 Future Improvements**

| **Feature**                               | **Priority** |
| ----------------------------------------- | ------------ |
| Implement indexed rendering (EBO)         | 🔥 High      |
| Add lighting calculations (Phong shading) | 🔥 High      |
| Support multiple objects in a scene       | 🚀 Medium    |
| Implement keyboard and mouse controls     | 🚀 Medium    |
| Load meshes from `.obj` format            | 🎯 Low       |

---

## **🚀 Summary**

- ✅ **Game engine successfully initializes SDL and OpenGL.**
- ✅ **Meshes load correctly from **``** and store positions in **``**.**
- ✅ **VAO and VBO are correctly bound and used for rendering.**
- ✅ **Shaders correctly process positions and normals.**
- ✅ **The rendered object is now visible on screen.**

🚀 **Next Steps: Implement advanced rendering features like lighting and camera movement!** 🚀

