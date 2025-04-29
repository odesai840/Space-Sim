#include "Sphere.h"
#include <iostream>
#include <cmath>
#include <glm/ext/scalar_constants.hpp>

namespace SpaceSim {

Sphere::Sphere(float radius, uint32_t latitudeBands, uint32_t longitudeBands)
    : m_Radius(radius), m_LatitudeBands(latitudeBands), m_LongitudeBands(longitudeBands),
      m_VAO(0), m_VBO(0), m_EBO(0), m_IndexCount(0)
{
    GenerateMesh();
}

Sphere::~Sphere()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void Sphere::GenerateMesh()
{
    m_Positions.clear();
    m_Normals.clear();
    m_Indices.clear();
    
    for (uint32_t lat = 0; lat <= m_LatitudeBands; lat++) {
        float theta = lat * glm::pi<float>() / m_LatitudeBands;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);
        
        for (uint32_t lon = 0; lon <= m_LongitudeBands; lon++) {
            float phi = lon * 2 * glm::pi<float>() / m_LongitudeBands;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);
            
            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;
            
            m_Positions.push_back(glm::vec3(m_Radius * x, m_Radius * y, m_Radius * z));

            m_Normals.push_back(glm::vec3(x, y, z));
        }
    }
    
    for (uint32_t lat = 0; lat < m_LatitudeBands; lat++) {
        for (uint32_t lon = 0; lon < m_LongitudeBands; lon++) {
            uint32_t first = lat * (m_LongitudeBands + 1) + lon;
            uint32_t second = first + m_LongitudeBands + 1;
            
            m_Indices.push_back(first);
            m_Indices.push_back(second);
            m_Indices.push_back(first + 1);
            
            m_Indices.push_back(second);
            m_Indices.push_back(second + 1);
            m_Indices.push_back(first + 1);
        }
    }
    
    m_IndexCount = static_cast<uint32_t>(m_Indices.size());
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    size_t vertexBufferSize = m_Positions.size() * (sizeof(glm::vec3) + sizeof(glm::vec3));
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, nullptr, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, m_Positions.size() * sizeof(glm::vec3), m_Positions.data());

    glBufferSubData(GL_ARRAY_BUFFER, m_Positions.size() * sizeof(glm::vec3), 
                    m_Normals.size() * sizeof(glm::vec3), m_Normals.data());
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 
                          (void*)(m_Positions.size() * sizeof(glm::vec3)));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32_t), 
                 m_Indices.data(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
}

void Sphere::Draw() const
{
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

}