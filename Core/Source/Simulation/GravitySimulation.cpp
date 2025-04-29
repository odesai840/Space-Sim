#include "GravitySimulation.h"
#include <random>
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace SpaceSim {

GravitySimulation::GravitySimulation()
{
    m_Shader = std::make_unique<Shader>();
}

void GravitySimulation::Init()
{
    const std::string vertexShaderSrc = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        uniform mat4 u_Model;
        uniform mat4 u_View;
        uniform mat4 u_Projection;
        
        out vec3 FragPos;
        out vec3 Normal;
        
        void main()
        {
            FragPos = vec3(u_Model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(u_Model))) * aNormal;
            gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
        }
    )";
    
    const std::string fragmentShaderSrc = R"(
        #version 460 core
        in vec3 FragPos;
        in vec3 Normal;
        
        uniform vec4 u_Color;
        uniform vec3 u_LightPos;
        uniform vec3 u_LightColor;
        uniform float u_AmbientStrength;
        uniform bool u_IsSun;
        
        out vec4 FragColor;
        
        void main()
        {
            if (u_IsSun) {
                FragColor = u_Color;
            }
            else {
                vec3 ambient = u_AmbientStrength * u_LightColor;
                
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(u_LightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * u_LightColor;
                
                vec3 result = (ambient + diffuse) * u_Color.rgb;
                FragColor = vec4(result, u_Color.a);
            }
        }
    )";
    
    m_Shader->Load(vertexShaderSrc, fragmentShaderSrc);
    
    Reset();
}

void GravitySimulation::Update(float deltaTime, float gravityStrength)
{
    for (auto& body : m_Bodies) {
        body->Update(m_Bodies, deltaTime, gravityStrength);
    }

    for (size_t i = 0; i < m_Bodies.size(); i++) {
        for (size_t j = i + 1; j < m_Bodies.size(); j++) {
            if (m_Bodies[i]->CheckCollision(m_Bodies[j])) {
                m_Bodies[i]->ResolveCollision(m_Bodies[j]);
            }
        }
    }
}

void GravitySimulation::Render(const glm::mat4& view, const glm::mat4& projection)
{
    glm::vec3 sunPosition = m_Bodies.empty() ? glm::vec3(0.0f) : m_Bodies[0]->GetPosition();
    
    for (size_t i = 0; i < m_Bodies.size(); i++) {
        m_Shader->Bind();
        
        m_Shader->SetVec4("u_Color", m_Bodies[i]->GetColor());
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), m_Bodies[i]->GetPosition());
        m_Shader->SetMat4("u_Model", model);
        m_Shader->SetMat4("u_View", view);
        m_Shader->SetMat4("u_Projection", projection);
        
        m_Shader->SetVec3("u_LightPos", sunPosition);
        m_Shader->SetVec3("u_LightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        m_Shader->SetFloat("u_AmbientStrength", 0.3f);
        
        m_Shader->SetBool("u_IsSun", i == 0);
        
        m_Bodies[i]->DrawMesh();
    }
}

GravitySimulation::OrbitParameters GravitySimulation::GetRandomOrbitParameters(float gravityStrength)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distRadius(3.0f, 15.0f);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * glm::pi<float>());
    std::uniform_real_distribution<float> distInclination(-0.3f, 0.3f);
    
    OrbitParameters params;
    
    params.distance = distRadius(gen);
    
    const float sunMass = 1000.0f;
    params.speed = std::sqrt(gravityStrength * sunMass / params.distance);
    
    params.angle = distAngle(gen);
    
    params.inclination = distInclination(gen);
    
    return params;
}

void GravitySimulation::AddRandomPlanet()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distRadius(0.2f, 0.6f);
    std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
    std::uniform_real_distribution<float> distColorType(0.0f, 1.0f);
    
    OrbitParameters orbit = GetRandomOrbitParameters();
    
    glm::vec3 position(
        orbit.distance * std::cos(orbit.angle),
        orbit.distance * std::sin(orbit.inclination),
        orbit.distance * std::sin(orbit.angle)
    );
    
    glm::vec3 velocity(
        -orbit.speed * std::sin(orbit.angle),
        orbit.speed * std::cos(orbit.inclination) * 0.1f,
        orbit.speed * std::cos(orbit.angle)
    );
    
    float radius = distRadius(gen);
    float mass = radius * radius * radius * 10.0f;
    
    glm::vec4 color;
    if (distColorType(gen) > 0.5f) {
        float blue = 0.5f + distColor(gen) * 0.5f;
        float green = 0.3f + distColor(gen) * 0.7f;
        color = glm::vec4(0.0f, green, blue, 1.0f);
    } else {
        float red = 0.7f + distColor(gen) * 0.3f;
        float green = 0.2f + distColor(gen) * 0.3f;
        color = glm::vec4(red, green, 0.1f, 1.0f);
    }
    
    m_Bodies.push_back(std::make_shared<CelestialBody>(radius, color, position, velocity, mass));
}

void GravitySimulation::AddPlanetWithParams(float distance, float angle, float radius, const glm::vec4& color)
{
    const float sunMass = 1000.0f;
    
    const float orbitSpeed = std::sqrt(1.0f * sunMass / distance);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distInclination(-0.2f, 0.2f);
    float inclination = distInclination(gen);
    
    glm::vec3 position(
        distance * std::cos(angle),
        distance * std::sin(inclination),
        distance * std::sin(angle)
    );
    
    glm::vec3 velocity(
        -orbitSpeed * std::sin(angle),
        orbitSpeed * std::cos(inclination) * 0.1f,
        orbitSpeed * std::cos(angle)
    );
    
    float mass = radius * radius * radius * 10.0f;
    
    m_Bodies.push_back(std::make_shared<CelestialBody>(radius, color, position, velocity, mass));
}

void GravitySimulation::Reset()
{
    m_Bodies.clear();
    
    m_Bodies.push_back(std::make_shared<CelestialBody>(
        1.5f,
        glm::vec4(1.0f, 0.8f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        1000.0f
    ));
    
    AddPlanetWithParams(4.0f, 0.0f, 0.3f, glm::vec4(0.2f, 0.7f, 0.9f, 1.0f));
    AddPlanetWithParams(7.0f, glm::pi<float>()/3.0f, 0.4f, glm::vec4(0.8f, 0.4f, 0.2f, 1.0f));
    AddPlanetWithParams(10.0f, glm::pi<float>()*2.0f/3.0f, 0.35f, glm::vec4(0.3f, 0.8f, 0.3f, 1.0f));
    AddPlanetWithParams(13.0f, glm::pi<float>(), 0.5f, glm::vec4(0.9f, 0.2f, 0.2f, 1.0f));
    AddPlanetWithParams(16.0f, glm::pi<float>()*4.0f/3.0f, 0.25f, glm::vec4(0.6f, 0.6f, 0.8f, 1.0f));
}

}