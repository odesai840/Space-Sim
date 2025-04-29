#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include <vector>
#include <memory>
#include <Glad/gl.h>
#include "Shader.h"

namespace SpaceSim {

class Skybox {
public:
    Skybox();
    ~Skybox();

    void Init(const std::vector<std::string>& faces);
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    uint32_t m_VAO;
    uint32_t m_VBO;
    uint32_t m_TextureID;
    std::unique_ptr<Shader> m_Shader;

    void LoadCubemap(const std::vector<std::string>& faces);
};

}

#endif