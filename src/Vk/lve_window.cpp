#include "lve_window.hpp"

// std
#include <stdexcept>

namespace Vk
{

    LveWindow::LveWindow(int width, int height, std::string name):
        m_width(width), m_height(height), m_window_name(name)
    {
        initWindow();
    }

    LveWindow::~LveWindow()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void LveWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window = glfwCreateWindow(m_width, m_height, m_window_name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if(glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void LveWindow::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
        lveWindow->frameBufferResized = true;
        lveWindow->m_width = width;
        lveWindow->m_height = height;
    }


}