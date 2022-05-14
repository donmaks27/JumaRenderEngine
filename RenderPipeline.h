// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "RenderEngineContextObject.h"

#include "jutils/jmap.h"
#include "jutils/jset.h"
#include "jutils/jstringID.h"

namespace JumaRenderEngine
{
    class Material;
    class VertexBuffer;
    struct RenderOptions;
    class RenderTarget;

    struct RenderPrimitive
    {
        VertexBuffer* vertexBuffer = nullptr;
        Material* material = nullptr;
    };
    struct RenderPipelineStage
    {
        RenderTarget* renderTarget = nullptr;
        jset<jstringID> dependencies;

        jarray<RenderPrimitive> renderPrimitives;
    };
    struct RenderPipelineStageQueueEntry
    {
        jstringID stage = jstringID_NONE;
        jarray<jstringID> stagesForSynchronization;
    };

    class RenderPipeline : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderPipeline() = default;
        virtual ~RenderPipeline() override = default;

        bool isPipelineQueueValid() const { return m_PipelineStagesQueueValid; }
        const jarray<RenderPipelineStageQueueEntry>& getPipelineQueue() const { return m_PipelineStagesQueue; }
        bool buildPipelineQueue();

        const jmap<jstringID, RenderPipelineStage>& getPipelineStages() const { return m_PipelineStages; }
        const RenderPipelineStage* getPipelineStage(const jstringID& stageName) const { return m_PipelineStages.find(stageName); }

        bool addPipelineStage(const jstringID& stageName, RenderTarget* renderTarget);
        void removePipelineStage(const jstringID& stageName);
        bool addPipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName);
        void removePipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName);

        bool addRenderPrimitive(const jstringID& stageName, const RenderPrimitive& primitive);
        void clearRenderPrimitives();

        bool render();

    protected:

        virtual bool init() { return true; }

        virtual void onStartRender();
        virtual void onFinishRender();

    private:

        jmap<jstringID, RenderPipelineStage> m_PipelineStages;

        bool m_PipelineStagesQueueValid = false;
        jarray<RenderPipelineStageQueueEntry> m_PipelineStagesQueue;
    };
}
