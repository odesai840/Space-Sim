#include "Application.h"
#include <iostream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace SpaceSim {

Application::Application(const std::string& title, uint32_t width, uint32_t height)
    : m_Title(title), m_Width(width), m_Height(height), m_Window(nullptr)
{
    s_Instance = this;
    Init();
}

Application::~Application()
{
    Shutdown();
}

void Application::Init()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);
    glfwMaximizeWindow(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(m_Window, MouseMoveCallback);
    glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
    glfwSetScrollCallback(m_Window, ScrollCallback);
    
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return;
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    
    InitImGui();
    
    m_Simulation = std::make_unique<GravitySimulation>();
    m_Simulation->Init();
}

void Application::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    
    ImGui::StyleColorsDark();
}

void Application::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Application::Run()
{
    float lastFrame = 0.0f;
    
    while (!glfwWindowShouldClose(m_Window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        ProcessInput();
        
        Update(deltaTime);
        
        Render();
        RenderUI();
        
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

void Application::ProcessInput()
{
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);
}

void Application::Update(float deltaTime)
{
    float scaledDeltaTime = deltaTime * m_TimeScale;
    
    m_Simulation->Update(scaledDeltaTime, m_GravityStrength);
}

void Application::Render()
{
    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::vec3 cameraPos;
    cameraPos.x = m_CameraDistance * std::sin(m_CameraRotationY) * std::cos(m_CameraRotationX);
    cameraPos.y = m_CameraDistance * std::sin(m_CameraRotationX);
    cameraPos.z = m_CameraDistance * std::cos(m_CameraRotationY) * std::cos(m_CameraRotationX);
    
    glm::mat4 view = glm::lookAt(
        cameraPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(m_Width) / static_cast<float>(m_Height),
        0.1f,
        100.0f
    );
    
    m_Simulation->Render(view, projection);
}

void Application::RenderUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::Begin("Simulation Controls");
    
    ImGui::Text("Gravity Strength");
    ImGui::SliderFloat("##GravityStrength", &m_GravityStrength, 0.0f, 2.0f, "%.2f");
    
    ImGui::Text("Time Scale");
    ImGui::SliderFloat("##TimeScale", &m_TimeScale, 0.0f, 2.0f, "%.2f");
    
    if (ImGui::Button("Add Random Planet"))
    {
        m_Simulation->AddRandomPlanet();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Simulation"))
    {
        m_Simulation->Reset();
    }
    
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::OnMouseMove(double xpos, double ypos)
{
    if (m_IsDragging)
    {
        float deltaX = static_cast<float>(xpos - m_LastMouseX);
        float deltaY = static_cast<float>(ypos - m_LastMouseY);
        
        m_CameraRotationY += deltaX * 0.01f;
        m_CameraRotationX += deltaY * 0.01f;
        
        m_CameraRotationX = glm::clamp(m_CameraRotationX, -glm::pi<float>() / 2.0f + 0.1f, glm::pi<float>() / 2.0f - 0.1f);
    }
    
    m_LastMouseX = xpos;
    m_LastMouseY = ypos;
}

void Application::OnMouseButton(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS && !ImGui::IsAnyItemHovered())
        {
            m_IsDragging = true;
        }
        else if (action == GLFW_RELEASE && !ImGui::IsAnyItemHovered())
        {
            m_IsDragging = false;
        }
    }
}

void Application::OnScroll(double xoffset, double yoffset)
{
    m_CameraDistance -= static_cast<float>(yoffset) * 1.0f;
    m_CameraDistance = glm::clamp(m_CameraDistance, 5.0f, 50.0f);
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    s_Instance->m_Width = width;
    s_Instance->m_Height = height;
}

void Application::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    s_Instance->OnMouseMove(xpos, ypos);
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    s_Instance->OnMouseButton(button, action, mods);
}

void Application::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    s_Instance->OnScroll(xoffset, yoffset);
}

}