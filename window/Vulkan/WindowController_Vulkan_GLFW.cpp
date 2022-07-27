// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_Vulkan_GLFW.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN) && defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)

#include <GLFW/glfw3.h>

#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    WindowController_Vulkan_GLFW::~WindowController_Vulkan_GLFW()
    {
        clearGLFW();
    }

    jarray<const char*> WindowController_Vulkan_GLFW::getVulkanInstanceExtensions() const
    {
        uint32 extensionsCount = 0;
        const char** extenstions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        if (extensionsCount == 0)
        {
            return {};
        }

        jarray<const char*> result(static_cast<int32>(extensionsCount));
        for (int32 index = 0; index < result.getSize(); index++)
        {
            result[index] = extenstions[index];
        }
        return result;
    }

    bool WindowController_Vulkan_GLFW::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }

        if (glfwInit() == GLFW_FALSE)
        {
#ifndef JUTILS_LOG_DISABLED
            const char* errorStr = nullptr;
            glfwGetError(&errorStr);
            JUMA_RENDER_LOG(error, jstring(JSTR("Failed to initialize GLFW lib: ")) + errorStr);
#endif
            return false;
        }

        glfwSetErrorCallback(WindowController_Vulkan_GLFW::GLFW_ErrorCallback);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return true;
    }
    void WindowController_Vulkan_GLFW::GLFW_ErrorCallback(const int errorCode, const char* errorMessage)
    {
        JUMA_RENDER_LOG(error, JSTR("GLFW error. Code: ") + TO_JSTR(errorCode) + JSTR(". ") + errorMessage);
    }

    void WindowController_Vulkan_GLFW::clearGLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows)
            {
                clearWindowGLFW(window.key, window.value);
            }
            m_Windows.clear();
        }

        glfwTerminate();
    }

    WindowData* WindowController_Vulkan_GLFW::createWindowInternal(const window_id windowID, const WindowProperties& properties)
    {
        if (windowID == window_id_INVALID)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid window ID"));
            return nullptr;
        }
        if (m_Windows.contains(windowID))
        {
            JUMA_RENDER_LOG(error, JSTR("Window ") + TO_JSTR(windowID) + JSTR(" already created"));
            return nullptr;
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(properties.size.x), static_cast<int>(properties.size.y), *properties.title, nullptr, nullptr
        );
        if (window == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create window ") + TO_JSTR(windowID));
            return nullptr;
        }

        VkSurfaceKHR surface = nullptr;
        const VkResult result = glfwCreateWindowSurface(getRenderEngine<RenderEngine_Vulkan>()->getVulkanInstance(), window, nullptr, &surface);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create surface for window ") + TO_JSTR(windowID));
            glfwDestroyWindow(window);
            return nullptr;
        }

        WindowData_Vulkan_GLFW& windowData = m_Windows[windowID];
        windowData.vulkanSurface = surface;
        windowData.windowGLFW = window;
        windowData.windowController = this;
        glfwSetWindowUserPointer(window, &windowData);
        glfwSetFramebufferSizeCallback(window, WindowController_Vulkan_GLFW::GLFW_FramebufferResizeCallback);

        if (!createWindowSwapchain(windowID, windowData))
        {
            clearWindowGLFW(windowID, windowData);
            m_Windows.remove(windowID);
            return nullptr;
        }
        return &windowData;
    }
    void WindowController_Vulkan_GLFW::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, const int width, const int height)
    {
        const WindowData_Vulkan_GLFW* windowData = static_cast<WindowData_Vulkan_GLFW*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            windowData->windowController->m_ChangedWindowSizes.add(windowData->windowID, { math::max<uint32>(width, 0), math::max<uint32>(height, 0) });
        }
    }

    void WindowController_Vulkan_GLFW::destroyWindow(const window_id windowID)
    {
        WindowData_Vulkan_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return;
        }

        clearWindowGLFW(windowID, *windowData);
        m_Windows.remove(windowID);
    }
    void WindowController_Vulkan_GLFW::clearWindowGLFW(const window_id windowID, WindowData_Vulkan_GLFW& windowData)
    {
        clearWindowVulkan(windowID, windowData);

        glfwSetWindowUserPointer(windowData.windowGLFW, nullptr);
        glfwDestroyWindow(windowData.windowGLFW);
        windowData.windowGLFW = nullptr;
        windowData.windowController = nullptr;
    }

    bool WindowController_Vulkan_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_Vulkan_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return false;
        }
        return glfwWindowShouldClose(windowData->windowGLFW) != GLFW_FALSE;
    }

    void WindowController_Vulkan_GLFW::onFinishRender()
    {
        Super::onFinishRender();

        glfwPollEvents();

        if (!m_ChangedWindowSizes.isEmpty())
        {
            for (const auto& changedWindowSize : m_ChangedWindowSizes)
            {
                onWindowResized(changedWindowSize.key, changedWindowSize.value);
            }
            m_ChangedWindowSizes.clear();
        }
    }

    bool WindowController_Vulkan_GLFW::setWindowTitle(const window_id windowID, const jstring& title)
    {
        const WindowData_Vulkan_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return false;
        }

        glfwSetWindowTitle(windowData->windowGLFW, *title);
        return true;
    }
}

#endif
