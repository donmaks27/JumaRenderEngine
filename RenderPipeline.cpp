// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderPipeline.h"

#include "RenderEngine.h"
#include "RenderOptions.h"
#include "RenderTarget.h"
#include "VertexBuffer.h"
#include "window/WindowController.h"

namespace JumaRenderEngine
{
    RenderPipeline::~RenderPipeline()
    {
        clearData();
    }

    bool RenderPipeline::init()
    {
        if (!initInternal())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize render pipeline"));
            clearData();
            return false;
        }
        return true;
    }
    bool RenderPipeline::initInternal()
    {
        return true;
    }

    void RenderPipeline::clearData()
    {
        m_PipelineStages.clear();
        m_PipelineStagesQueueValid = false;
        m_PipelineStagesQueue.clear();
    }

    bool RenderPipeline::buildPipelineQueue()
    {
        if (m_PipelineStagesQueueValid)
        {
            return true;
        }
        if (m_PipelineStages.isEmpty())
        {
            return false;
        }

        m_PipelineStagesQueue.clear();
        jmap<jstringID, RenderPipelineStage> cachedPipelineStages = m_PipelineStages;
        jarray<jstringID> handledStages;
        jarray<jstringID> stagesForSync;
        while (!cachedPipelineStages.isEmpty())
        {
            // Get stages without synced dependencies
            for (const auto& stage : cachedPipelineStages)
            {
                if (stage.value.dependencies.isEmpty())
                {
                    handledStages.add(stage.key);
                }
            }
            if (handledStages.isEmpty())
            {
                JUMA_RENDER_LOG(error, JSTR("Failed validate render pipeline queue"));
                return false;
            }

            // Add them to queue
            for (const auto& stage : handledStages)
            {
                cachedPipelineStages.remove(stage);
                if (!stagesForSync.isEmpty())
                {
                    m_PipelineStagesQueue.add({ stage, stagesForSync });
                    stagesForSync.clear();
                }
                else
                {
                    m_PipelineStagesQueue.add({ stage, {} });
                }
            }

            // Remove them from dependencies and mark as needed to sync
            for (auto& stage : cachedPipelineStages)
            {
                for (const auto& handledStage : handledStages)
                {
                    stage.value.dependencies.remove(handledStage);
                }
            }
            stagesForSync = handledStages;
            handledStages.clear();
        }

        m_PipelineStagesQueueValid = true;
        return true;
    }

    bool RenderPipeline::addPipelineStage(const jstringID& stageName, RenderTarget* renderTarget)
    {
        if ((stageName == jstringID_NONE) || (renderTarget == nullptr))
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid input params"));
            return false;
        }
        if (m_PipelineStages.contains(stageName))
        {
            JUMA_RENDER_LOG(error, JSTR("Stage already exists"));
            return false;
        }

        RenderPipelineStage& newStage = m_PipelineStages.add(stageName);
        newStage.renderTarget = renderTarget;
        m_PipelineStagesQueueValid = false;
        return true;
    }
    void RenderPipeline::removePipelineStage(const jstringID& stageName)
    {
        const RenderPipelineStage* stage = m_PipelineStages.find(stageName);
        if (stage != nullptr)
        {
            m_PipelineStages.remove(stageName);
            for (auto& pipelineStage : m_PipelineStages)
            {
                pipelineStage.value.dependencies.remove(stageName);
            }
            m_PipelineStagesQueueValid = false;
        }
    }

    bool RenderPipeline::addPipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName)
    {
        if (dependencyStageName == jstringID_NONE)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid dependency name"));
            return false;
        }

        RenderPipelineStage* stage = m_PipelineStages.find(stageName);
        if (stage == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("There is no stage ") + stageName.toString());
            return false;
        }
        if (stage->dependencies.contains(dependencyStageName))
        {
            return true;
        }

        const RenderPipelineStage* dependencyStage = m_PipelineStages.find(dependencyStageName);
        if (dependencyStage == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("There is no dependency stage ") + dependencyStageName.toString());
            return false;
        }
        if (dependencyStage->renderTarget->isWindowRenderTarget())
        {
            JUMA_RENDER_LOG(error, JSTR("You can't set window stage as dependency"));
            return false;
        }

        stage->dependencies.add(dependencyStageName);
        m_PipelineStagesQueueValid = false;
        return true;
    }
    void RenderPipeline::removePipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName)
    {
        RenderPipelineStage* stage = m_PipelineStages.find(stageName);
        if ((stage != nullptr) && stage->dependencies.remove(dependencyStageName))
        {
            m_PipelineStagesQueueValid = false;
        }
    }

    bool RenderPipeline::addRenderPrimitive(const jstringID& stageName, const RenderPrimitive& primitive)
    {
        RenderPipelineStage* stage = m_PipelineStages.find(stageName);
        if (stage == nullptr)
        {
            return false;
        }
        stage->renderPrimitives.add(primitive);
        return true;
    }
    void RenderPipeline::clearRenderPrimitives()
    {
        for (auto& stage : m_PipelineStages)
        {
            stage.value.renderPrimitives.clear();
        }
    }

    bool RenderPipeline::render()
    {
        if (!isPipelineQueueValid())
        {
            return false;
        }
        renderInternal();
        return true;
    }
    void RenderPipeline::renderInternal()
    {
        callRender<RenderOptions>();
    }
    void RenderPipeline::callRender(RenderOptions* renderOptions)
    {
        renderOptions->renderPipeline = this;
        if (onStartRender(renderOptions))
        {
            for (const auto& renderQueueEntry : getPipelineQueue())
            {
                const RenderPipelineStage* pipelineStage = getPipelineStage(renderQueueEntry.stage);
                renderOptions->renderTarget = pipelineStage->renderTarget;
                if (!pipelineStage->renderTarget->onStartRender(renderOptions))
                {
                    continue;
                }

                for (const auto& renderPrimitive : pipelineStage->renderPrimitives)
                {
                    renderPrimitive.vertexBuffer->render(renderOptions, renderPrimitive.material);
                }
                pipelineStage->renderTarget->onFinishRender(renderOptions);
            }

            onFinishRender(renderOptions);
        }
    }

    bool RenderPipeline::onStartRender(RenderOptions* renderOptions)
    {
        return getRenderEngine()->getWindowController()->onStartRender();
    }
    void RenderPipeline::onFinishRender(RenderOptions* renderOptions)
    {
        getRenderEngine()->getWindowController()->onFinishRender();
    }
}
