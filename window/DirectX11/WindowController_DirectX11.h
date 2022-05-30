// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/window/WindowController.h"

namespace JumaRenderEngine
{
    struct WindowData_DirectX11 : WindowData
    {
        
    };

    class WindowController_DirectX11 : public WindowController
    {
        using Super = WindowController;

    public:
        WindowController_DirectX11() = default;
        virtual ~WindowController_DirectX11() override;

    private:

        void clearDirectX11();
    };
}

#endif
