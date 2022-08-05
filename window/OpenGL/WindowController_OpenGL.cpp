// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_OpenGL.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include <GL/glew.h>

namespace JumaRenderEngine
{
    bool WindowController_OpenGL::initOpenGL()
    {
        const GLenum glewInitResult = glewInit();
        if (glewInitResult != GLEW_OK)
        {
            JUMA_RENDER_LOG(error, reinterpret_cast<const char*>(glewGetErrorString(glewInitResult)));
            return false;
        }
        return true;
    }
    
    void WindowController_OpenGL::setActiveWindowID(const window_id windowID)
    {
        if (windowID != m_ActiveWindowID)
        {
            if (setActiveWindowInternal(windowID))
            {
                m_ActiveWindowID = windowID;
            }
            else
            {
                JUMA_RENDER_LOG(warning, JSTR("Failed to set active window ID {}"), windowID);
            }
        }
    }
}

#endif