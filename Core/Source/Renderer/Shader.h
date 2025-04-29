#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>
#include <Glad/gl.h>
#include <glm/glm.hpp>

namespace SpaceSim {

class Shader {
public:
    Shader();
    ~Shader();

    void Load(const std::string& vertexSrc, const std::string& fragmentSrc);
    void LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    
    void Bind() const;
    void Unbind() const;
    
    void SetBool(const std::string& name, bool value);
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat2(const std::string& name, const glm::mat2& value);
    void SetMat3(const std::string& name, const glm::mat3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);

private:
    uint32_t m_RendererID;
    std::unordered_map<std::string, int> m_UniformLocations;

    int GetUniformLocation(const std::string& name);
    void CheckCompileErrors(unsigned int shader, const std::string& type);
};

}

#endif