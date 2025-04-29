#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <memory>
#include <Glad/gl.h>
#include <GLFW/glfw3.h>
#include "Simulation/GravitySimulation.h"

namespace SpaceSim {

class Application {
public:
    Application(const std::string& title = "Space Simulation", uint32_t width = 1920, uint32_t height = 1080);
    ~Application();

    void Run();

private:
    void Init();
    void InitImGui();
    void Shutdown();
    void ProcessInput();
    void Update(float deltaTime);
    void Render();
    void RenderUI();
    
    void OnMouseMove(double xpos, double ypos);
    void OnMouseButton(int button, int action, int mods);
    void OnScroll(double xoffset, double yoffset);
    
    std::string m_Title;
    uint32_t m_Width;
    uint32_t m_Height;
    GLFWwindow* m_Window;
    
    float m_CameraDistance = 20.0f;
    float m_CameraRotationX = 0.3f;
    float m_CameraRotationY = 0.0f;
    bool m_IsDragging = false;
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;
    
    float m_GravityStrength = 1.0f;
    float m_TimeScale = 1.0f;
    bool m_PauseSimulation = false;
    float m_NewPlanetDistance = 8.0f;
    float m_NewPlanetAngle = 0.0f;
    float m_NewPlanetRadius = 0.3f;
    glm::vec4 m_NewPlanetColor = glm::vec4(0.5f, 0.5f, 0.9f, 1.0f);
    
    struct CameraPreset {
        float distance;
        float rotationX;
        float rotationY;
        std::string name;
    };
    std::vector<CameraPreset> m_CameraPresets;
    
    std::unique_ptr<GravitySimulation> m_Simulation;
    
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    static inline Application* s_Instance = nullptr;
};

}

#endif