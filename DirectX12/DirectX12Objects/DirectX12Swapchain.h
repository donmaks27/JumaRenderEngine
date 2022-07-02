// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderEngineContextObject.h"

#include "renderEngine/window/window_id.h"

struct IDXGISwapChain4;

namespace JumaRenderEngine
{
    class WindowController_DirectX12;

    class DirectX12Swapchain : public RenderEngineContextObject
    {
        friend WindowController_DirectX12;

    public:
        DirectX12Swapchain() = default;
        virtual ~DirectX12Swapchain() override;

        IDXGISwapChain4* get() const { return m_Swapchain; }

        bool present();

    private:

        IDXGISwapChain4* m_Swapchain = nullptr;

        uint32 m_CurrentBufferIndex = 0;


        bool init(window_id windowID);

        void clearDirectX();
    };
}

#endif
