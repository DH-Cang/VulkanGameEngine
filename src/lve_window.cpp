#include "lve_window.hpp"

// std
#include <stdexcept>

namespace lve
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
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(m_width, m_height, m_window_name.c_str(), nullptr, nullptr);

    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if(glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

}