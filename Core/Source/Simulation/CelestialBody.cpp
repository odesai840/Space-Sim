#include "CelestialBody.h"
#include <glm/gtc/matrix_transform.hpp>

namespace SpaceSim {

CelestialBody::CelestialBody(float radius, const glm::vec4& color, const glm::vec3& position, const glm::vec3& velocity, float mass)
    : m_Radius(radius), m_Color(color), m_Position(position), m_Velocity(velocity), m_Mass(mass)
{
    uint32_t detail = static_cast<uint32_t>(glm::max(10.0f, glm::min(30.0f, radius * 5.0f)));
    m_Sphere = std::make_shared<Sphere>(radius, detail, detail);
}

void CelestialBody::Update(const std::vector<std::shared_ptr<CelestialBody>>& bodies, float deltaTime, float gravityStrength)
{
    for (const auto& otherBody : bodies) {
        if (otherBody.get() == this) continue;
        
        glm::vec3 direction = otherBody->GetPosition() - m_Position;
        float distance = glm::length(direction);
        
        if (distance < 0.1f) continue;
        
        float forceMagnitude = gravityStrength * m_Mass * otherBody->GetMass() / (distance * distance);
        
        glm::vec3 acceleration = glm::normalize(direction) * (forceMagnitude / m_Mass);
        
        m_Velocity += acceleration * deltaTime;
    }
    
    m_Position += m_Velocity * deltaTime;
}

void CelestialBody::DrawMesh() const
{
    m_Sphere->Draw();
}

bool CelestialBody::CheckCollision(const std::shared_ptr<CelestialBody>& other) const
{
    glm::vec3 direction = other->GetPosition() - m_Position;
    float distance = glm::length(direction);
    float minDistance = m_Radius + other->GetRadius();
    
    return distance < minDistance;
}

void CelestialBody::ResolveCollision(std::shared_ptr<CelestialBody>& other)
{
    glm::vec3 direction = other->GetPosition() - m_Position;
    float distance = glm::length(direction);
    
    if (distance == 0.0f)
    {
        direction = glm::vec3(0.001f, 0.0f, 0.0f);
        distance = 0.001f;
    }
    
    glm::vec3 normal = direction / distance;
    
    float overlap = (m_Radius + other->GetRadius()) - distance;
    m_Position -= normal * (overlap * 0.5f);
    other->SetPosition(other->GetPosition() + normal * (overlap * 0.5f));
    
    glm::vec3 relativeVelocity = other->GetVelocity() - m_Velocity;
    
    float velocityAlongNormal = glm::dot(relativeVelocity, normal);
    
    if (velocityAlongNormal > 0)
        return;
    
    float restitution = 0.8f;
    
    float impulseScalar = -(1.0f + restitution) * velocityAlongNormal;
    impulseScalar /= (1.0f / m_Mass) + (1.0f / other->GetMass());
    
    glm::vec3 impulse = normal * impulseScalar;
    m_Velocity -= impulse / m_Mass;
    other->SetVelocity(other->GetVelocity() + impulse / other->GetMass());
}

}