#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Renderer/Shader.h"
#include "Renderer/Sphere.h"

namespace SpaceSim {

class CelestialBody {
public:
    CelestialBody(float radius, const glm::vec4& color, const glm::vec3& position, const glm::vec3& velocity, float mass);
    ~CelestialBody() = default;
    
    void Update(const std::vector<std::shared_ptr<CelestialBody>>& bodies, float deltaTime, float gravityStrength);
    void DrawMesh() const;
    
    float GetRadius() const { return m_Radius; }
    float GetMass() const { return m_Mass; }
    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetVelocity() const { return m_Velocity; }
    const glm::vec4& GetColor() const { return m_Color; }
    
    void SetPosition(const glm::vec3& position) { m_Position = position; }
    void SetVelocity(const glm::vec3& velocity) { m_Velocity = velocity; }

    bool CheckCollision(const std::shared_ptr<CelestialBody>& other) const;
    void ResolveCollision(std::shared_ptr<CelestialBody>& other);
    
private:
    float m_Radius;
    glm::vec4 m_Color;
    glm::vec3 m_Position;
    glm::vec3 m_Velocity;
    float m_Mass;
    
    std::shared_ptr<Sphere> m_Sphere;
};

}