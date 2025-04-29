#include "Application.h"
#include <iostream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <SOIL2/SOIL2.h>

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
    
    GLFWimage images[1];
    images[0].pixels = SOIL_load_image("../Assets/Branding/sockenginelogo.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(m_Window, 1, images);
    SOIL_free_image_data(images[0].pixels);
    
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
    float lastFrame = static_cast<float>(glfwGetTime());
    
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
    
    static bool spacePressed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!spacePressed) {
            m_PauseSimulation = !m_PauseSimulation;
            spacePressed = true;
        }
    } else {
        spacePressed = false;
    }
    
    static bool rPressed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!rPressed) {
            m_Simulation->Reset();
            rPressed = true;
        }
    } else {
        rPressed = false;
    }
    
    static bool aPressed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) {
        if (!aPressed) {
            m_Simulation->AddRandomPlanet();
            aPressed = true;
        }
    } else {
        aPressed = false;
    }
}

void Application::Update(float deltaTime)
{
    if (!m_PauseSimulation)
    {
        float scaledDeltaTime = deltaTime * m_TimeScale;
        m_Simulation->Update(scaledDeltaTime, m_GravityStrength);
    }
}

void Application::Render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

    if (ImGui::CollapsingHeader("Simulation Status", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Bodies: %zu", m_Simulation->GetBodyCount());
        
        ImGui::Checkbox("Pause Simulation", &m_PauseSimulation);
        
        if (ImGui::Button("Reset Simulation"))
        {
            m_Simulation->Reset();
        }
    }
    
    if (ImGui::CollapsingHeader("Simulation Parameters", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Gravity Strength");
        ImGui::SliderFloat("##GravityStrength", &m_GravityStrength, 0.0f, 5.0f, "%.2f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Adjusts the strength of gravity in the simulation");
        
        ImGui::Text("Time Scale");
        ImGui::SliderFloat("##TimeScale", &m_TimeScale, 0.0f, 5.0f, "%.2f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Adjusts the speed of the simulation");
        
        if (ImGui::Button("Reset Parameters")) {
            m_GravityStrength = 1.0f;
            m_TimeScale = 1.0f;
        }
    }
    
    if (ImGui::CollapsingHeader("Add Planet", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Add Random Planet"))
        {
            m_Simulation->AddRandomPlanet();
        }
        
        ImGui::Separator();
        
        ImGui::Text("Custom Planet Parameters:");
        
        ImGui::SliderFloat("Distance from Sun", &m_NewPlanetDistance, 3.0f, 20.0f, "%.1f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Distance from the center of the system");
            
        ImGui::SliderFloat("Orbit Angle", &m_NewPlanetAngle, 0.0f, 6.28f, "%.2f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Initial position on the orbit (radians)");
            
        ImGui::SliderFloat("Planet Size", &m_NewPlanetRadius, 0.1f, 1.0f, "%.2f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Radius of the planet");
        
        ImGui::ColorEdit4("Planet Color", &m_NewPlanetColor.x);
        
        if (ImGui::Button("Add Custom Planet"))
        {
            m_Simulation->AddPlanetWithParams(
                m_NewPlanetDistance,
                m_NewPlanetAngle,
                m_NewPlanetRadius,
                m_NewPlanetColor
            );
        }
    }
    
    if (ImGui::CollapsingHeader("Camera Controls", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Camera Distance", &m_CameraDistance, 5.0f, 50.0f, "%.1f");
        
        if (ImGui::Button("Top View"))
        {
            m_CameraRotationX = -1.57f;
            m_CameraRotationY = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Side View"))
        {
            m_CameraRotationX = 0.0f;
            m_CameraRotationY = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Angled View"))
        {
            m_CameraRotationX = 0.3f;
            m_CameraRotationY = 0.0f;
        }
        
        ImGui::Text("Mouse Controls:");
        ImGui::BulletText("Left-click and drag to rotate camera");
        ImGui::BulletText("Scroll to zoom in/out");
    }
    
    if (ImGui::CollapsingHeader("Help", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::BulletText("Space: Pause/Resume simulation");
        ImGui::BulletText("R: Reset simulation");
        ImGui::BulletText("A: Add random planet");
        ImGui::BulletText("Escape: Exit application");
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
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
    {
        return;
    }
    
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            m_IsDragging = true;
        }
        else if (action == GLFW_RELEASE)
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