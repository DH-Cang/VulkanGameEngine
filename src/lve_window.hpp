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

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        void initWindow();

        const int m_width;
        const int m_height;

        std::string m_window_name;
        GLFWwindow *m_window;
    };
} // namespace lve