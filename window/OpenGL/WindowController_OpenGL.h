// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include "renderEngine/window/WindowController.h"

namespace JumaRenderEngine
{
    struct WindowData_OpenGL : WindowData
    {
    };

    class WindowController_OpenGL : public WindowController
    {
        using Super = WindowController;

    public:
        WindowController_OpenGL() = default;
        virtual ~WindowController_OpenGL() override = default;

        window_id getActiveWindowID() const { return m_ActiveWindowID; }
        void setActiveWindowID(window_id windowID);

    protected:

        virtual bool setActiveWindowInternal(window_id windowID) = 0;

        bool initOpenGL();

    private:

        window_id m_ActiveWindowID = window_id_INVALID;
    };
}

#endif
