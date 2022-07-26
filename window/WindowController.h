// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "renderEngine/RenderEngineContextObject.h"

#include "window_id.h"
#include "jutils/jarray.h"
#include "jutils/jdelegate_multicast.h"
#include "jutils/math/vector2.h"
#include "renderEngine/texture/TextureSamples.h"

namespace JumaRenderEngine
{
    class RenderTarget;
    class WindowController;

    struct WindowProperties
    {
        jstring title;
        math::uvector2 size;
        TextureSamples samples = TextureSamples::X1;
    };
    struct WindowData
    {
        window_id windowID = window_id_INVALID;
        WindowProperties properties;
        RenderTarget* windowRenderTarget = nullptr;
    };

    CREATE_JUTILS_MULTICAST_DELEGATE_TwoParams(OnWindowControllerWindowEvent, WindowController*, windowController, const WindowData*, windowData);

    class WindowController : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        WindowController() = default;
        virtual ~WindowController() override = default;

        OnWindowControllerWindowEvent OnWindowPropertiesChanged;


        bool createWindow(window_id windowID, const WindowProperties& properties);
        virtual void destroyWindow(window_id windowID) = 0;

        bool createRenderTargets();
        void clearRenderTargets();

        virtual const WindowData* findWindowData(window_id windowID) const = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        const T* findWindowData(const window_id windowID) const { return reinterpret_cast<const T*>(findWindowData(windowID)); }
        virtual jarray<window_id> getWindowIDs() const = 0;

        virtual bool shouldCloseWindow(window_id windowID) const = 0;

        virtual bool onStartRender() { return true; }
        virtual bool onStartWindowRender(window_id windowID) { return true; }
        virtual void onFinishWindowRender(window_id windowID) {}
        virtual void onFinishRender() {}

        virtual bool setWindowTitle(window_id windowID, const jstring& title) = 0;

    protected:

        virtual bool initWindowController() { return true; }

        virtual WindowData* createWindowInternal(window_id windowID, const WindowProperties& properties) = 0;

        void clearWindow(window_id windowID, WindowData& windowData);

        virtual WindowData* getWindowData(window_id windowID) = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        T* getWindowData(const window_id windowID) { return reinterpret_cast<T*>(getWindowData(windowID)); }

        void onWindowResized(window_id windowID, const math::uvector2& newSize);

    private:
        
        bool createRenderTarget(window_id windowID, WindowData& windowData);
        void clearRenderTarget(window_id windowID, WindowData& windowData);
    };
}
