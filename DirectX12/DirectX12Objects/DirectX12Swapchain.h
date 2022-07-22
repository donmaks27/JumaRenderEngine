// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderEngineContextObject.h"

#include "DirectX12Texture.h"
#include "jutils/jarray.h"
#include "renderEngine/window/window_id.h"

struct IDXGISwapChain4;

namespace JumaRenderEngine
{
    class WindowController_DirectX12;

    class DirectX12Swapchain : public RenderEngineContextObjectBase
    {
        friend WindowController_DirectX12;

    public:
        DirectX12Swapchain() = default;
        virtual ~DirectX12Swapchain() override;

        IDXGISwapChain4* get() const { return m_Swapchain; }

        DirectX12Texture* getBuffer(const uint8 bufferIndex) const { return m_SwapchainBuffers.findByIndex(bufferIndex); }
        uint8 getBuffersCount() const { return static_cast<uint8>(m_SwapchainBuffers.getSize()); }
        uint8 getCurrentBufferIndex() const { return m_CurrentBufferIndex; }

        bool present();

    private:

        IDXGISwapChain4* m_Swapchain = nullptr;

        mutable jarray<DirectX12Texture> m_SwapchainBuffers;
        uint8 m_CurrentBufferIndex = 0;


        bool init(window_id windowID);

        void clearDirectX();
    };
}

#endif
