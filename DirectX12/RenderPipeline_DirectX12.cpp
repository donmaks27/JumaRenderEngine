// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderPipeline_DirectX12.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "DirectX12Objects/DirectX12CommandList.h"
#include "DirectX12Objects/DirectX12Swapchain.h"
#include "renderEngine/window/DirectX12/WindowController_DirectX12.h"

namespace JumaRenderEngine
{
    RenderPipeline_DirectX12::~RenderPipeline_DirectX12()
    {
        clearDirectX();
    }

    void RenderPipeline_DirectX12::clearDirectX()
    {
        waitForPreviousRenderFinish();
    }

    void RenderPipeline_DirectX12::renderInternal()
    {
        callRender<RenderOptions_DirectX12>();
    }

    bool RenderPipeline_DirectX12::onStartRender(RenderOptions* renderOptions)
    {
        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }

        waitForPreviousRenderFinish();
        return startCommandListRecord(renderOptions);
    }
    void RenderPipeline_DirectX12::onFinishRender(RenderOptions* renderOptions)
    {
        finishCommandListRecord(renderOptions);
        Super::onFinishRender(renderOptions);
    }

    void RenderPipeline_DirectX12::waitForPreviousRenderFinish()
    {
        if (m_RenderCommandList != nullptr)
        {
            m_RenderCommandList->waitForFinish();
            m_RenderCommandList->markUnused();
            m_RenderCommandList = nullptr;
        }
    }
    bool RenderPipeline_DirectX12::startCommandListRecord(RenderOptions* renderOptions)
    {
        DirectX12CommandQueue* commandQueue = getRenderEngine<RenderEngine_DirectX12>()->getCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (commandQueue == nullptr)
        {
            return false;
        }
        m_RenderCommandList = commandQueue->getCommandList();
        reinterpret_cast<RenderOptions_DirectX12*>(renderOptions)->renderCommandList = m_RenderCommandList;
        return true;
    }
    void RenderPipeline_DirectX12::finishCommandListRecord(RenderOptions* renderOptions)
    {
        m_RenderCommandList->execute();

        const WindowController* windowController = getRenderEngine()->getWindowController();
        for (const auto& windowID : windowController->getWindowIDs())
        {
            const WindowData_DirectX12* windowData = windowController->findWindowData<WindowData_DirectX12>(windowID);
            if ((windowData == nullptr) || (windowData->swapchain == nullptr))
            {
                continue;
            }
            windowData->swapchain->present();
        }
    }
}

#endif
