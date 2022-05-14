// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    class RenderEngine;

    class RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderEngineContextObjectBase() = default;
        virtual ~RenderEngineContextObjectBase() = default;

        RenderEngine* getRenderEngine() const { return m_RenderEngine; }
        template<typename T, TEMPLATE_ENABLE(is_base<RenderEngine, T>)>
        T* getRenderEngine() const { return dynamic_cast<T*>(getRenderEngine()); }

    private:

        RenderEngine* m_RenderEngine = nullptr;
    };
    class RenderEngineContextObject : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderEngineContextObject() = default;
        virtual ~RenderEngineContextObject() override = default;

        bool isValid() const { return m_Initialized; }
        void clear()
        {
            if (isValid())
            {
                clearInternal();
                m_Initialized = false;
            }
        }

    protected:

        void markAsInitialized() { m_Initialized = true; }

        virtual void clearInternal() {}

    private:

        bool m_Initialized = false;
    };
}
