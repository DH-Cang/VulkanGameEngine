#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace Platform
{
    class MyWindow
    {
    public:
        MyWindow(int width, int height, std::string name);
        ~MyWindow();

        MyWindow(const MyWindow &) = delete;
        MyWindow& operator=(const MyWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(m_window); }
        VkExtent2D getExtent() { return VkExtent2D{static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_width)}; }
        bool wasWindowResized() { return frameBufferResized; }
        void resetWindowResizedFlag() { frameBufferResized = false; }
        GLFWwindow* getGLFWwindow() const { return m_window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void frameBufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);
        void initWindow();

        int m_width;
        int m_height;
        bool frameBufferResized = false;

        std::string m_window_name;
        GLFWwindow *m_window;
    };
}