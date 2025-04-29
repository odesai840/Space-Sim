#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <Glad/gl.h>
#include <glm/glm.hpp>

namespace SpaceSim {

class Sphere {
public:
    Sphere(float radius = 1.0f, uint32_t latitudeBands = 20, uint32_t longitudeBands = 20);
    ~Sphere();
    
    void Draw() const;
    
private:
    void GenerateMesh();
    
    float m_Radius;
    uint32_t m_LatitudeBands;
    uint32_t m_LongitudeBands;
    
    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<uint32_t> m_Indices;
    
    uint32_t m_VAO;
    uint32_t m_VBO;
    uint32_t m_EBO;
    uint32_t m_IndexCount;
};

}

#endif