#include "my_window.hpp"

// std
#include <stdexcept>

namespace Platform
{

    MyWindow::MyWindow(int width, int height, std::string name):
        m_width(width), m_height(height), m_window_name(name)
    {
        initWindow();
    }

    MyWindow::~MyWindow()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void MyWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window = glfwCreateWindow(m_width, m_height, m_window_name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
    }

    void MyWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if(glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void MyWindow::frameBufferResizeCallback(GLFWwindow* glfwWindow, int width, int height)
    {
        auto window = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(glfwWindow));
        window->frameBufferResized = true;
        window->m_width = width;
        window->m_height = height;
    }


}