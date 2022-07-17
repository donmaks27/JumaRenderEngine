// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderPipeline.h"

namespace JumaRenderEngine
{
    class DirectX12CommandList;

    class RenderPipeline_DirectX12 : public RenderPipeline
    {
        using Super = RenderPipeline;

    public:
        RenderPipeline_DirectX12() = default;
        virtual ~RenderPipeline_DirectX12() override;

    protected:

        virtual void renderInternal() override;

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    private:

        DirectX12CommandList* m_RenderCommandList = nullptr;


        void clearDirectX();

        void waitForPreviousRenderFinish();
        bool startCommandListRecord(RenderOptions* renderOptions);
        void finishCommandListRecord(RenderOptions* renderOptions);
    };
}

#endif
