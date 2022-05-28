// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController.h"

namespace JumaRenderEngine
{
    void WindowController::onWindowResized(const window_id windowID, const math::uvector2& newSize)
    {
        getWindowData(windowID)->size = newSize;
    }
}
