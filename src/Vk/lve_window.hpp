#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace lve
{
    class LveWindow
    {
    public:
        LveWindow(int width, int height, std::string name);
        ~LveWindow();

        LveWindow(const LveWindow &) = delete;
        LveWindow& operator=(const LveWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(m_window); }
        VkExtent2D getExtent() { return VkExtent2D{static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_width)}; }
        bool wasWindowResized() { return frameBufferResized; }
        void resetWindowResizedFlag() { frameBufferResized = false; }
        GLFWwindow* getGLFWwindow() const { return m_window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
        void initWindow();

        int m_width;
        int m_height;
        bool frameBufferResized = false;

        std::string m_window_name;
        GLFWwindow *m_window;
    };
} // namespace lve