#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Renderer/Shader.h"
#include "CelestialBody.h"

namespace SpaceSim {

class GravitySimulation {
public:
    GravitySimulation();
    ~GravitySimulation() = default;
    
    void Init();
    void Update(float deltaTime, float gravityStrength);
    void Render(const glm::mat4& view, const glm::mat4& projection);
    
    void AddRandomPlanet();
    void AddPlanetWithParams(float distance, float angle, float radius, const glm::vec4& color);
    void Reset();
    
private:
    std::vector<std::shared_ptr<CelestialBody>> m_Bodies;
    std::unique_ptr<Shader> m_Shader;
    
    struct OrbitParameters {
        float distance;
        float speed;
        float angle;
        float inclination;
    };
    
    OrbitParameters GetRandomOrbitParameters(float gravityStrength = 1.0f);
};

}