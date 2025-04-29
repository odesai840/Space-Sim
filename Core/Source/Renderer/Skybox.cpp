#include "Skybox.h"
#include <iostream>
#include <SOIL2/SOIL2.h>
#include <glm/ext/matrix_transform.hpp>

namespace SpaceSim {

Skybox::Skybox()
    : m_VAO(0), m_VBO(0), m_TextureID(0)
{
    m_Shader = std::make_unique<Shader>();
}

Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteTextures(1, &m_TextureID);
}

void Skybox::Init(const std::vector<std::string>& faces)
{
    // Skybox vertices (cube)
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    // Create VAO, VBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
    
    // Load cubemap textures
    LoadCubemap(faces);
    
    // Create shader
    const std::string vertexShaderSrc = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 u_View;
        uniform mat4 u_Projection;
        
        out vec3 TexCoords;
        
        void main()
        {
            TexCoords = aPos;
            vec4 pos = u_Projection * u_View * vec4(aPos, 1.0);
            gl_Position = pos.xyww; // Ensure skybox is always at maximum depth
        }
    )";
    
    const std::string fragmentShaderSrc = R"(
        #version 460 core
        in vec3 TexCoords;
        
        uniform samplerCube u_Skybox;
        
        out vec4 FragColor;
        
        void main()
        {    
            FragColor = texture(u_Skybox, TexCoords);
        }
    )";
    
    m_Shader->Load(vertexShaderSrc, fragmentShaderSrc);
}

void Skybox::Draw(const glm::mat4& view, const glm::mat4& projection)
{
    // Remove translation from view matrix for skybox
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    
    // Disable depth writing
    glDepthFunc(GL_LEQUAL);
    
    m_Shader->Bind();
    m_Shader->SetMat4("u_View", skyboxView);
    m_Shader->SetMat4("u_Projection", projection);
    m_Shader->SetInt("u_Skybox", 0);
    
    glBindVertexArray(m_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glBindVertexArray(0);
    
    // Reset depth function
    glDepthFunc(GL_LESS);
}

void Skybox::LoadCubemap(const std::vector<std::string>& faces)
{
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
    
    for (size_t i = 0; i < faces.size(); i++)
    {
        int width, height, channels;
        unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
        
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, 
                         channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
            SOIL_free_image_data(data);
        }
        else
        {
            std::cerr << "Failed to load cubemap texture: " << faces[i] << std::endl;
            SOIL_free_image_data(data);
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

}