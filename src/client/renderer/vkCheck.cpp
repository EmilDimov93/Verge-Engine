// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

namespace VE
{
    void Renderer::vkCheck(VkResult res, ErrorCode errorCode)
    {
        switch (res)
        {
        case VK_SUCCESS:
            return;
        case VK_NOT_READY:
            Log::add('R', 100);
            break;
        case VK_TIMEOUT:
            Log::add('R', 101);
            break;
        case VK_SUBOPTIMAL_KHR:
            Log::add('R', 102);
            break;
        case VK_EVENT_SET:
            Log::add('R', 103);
            break;
        case VK_EVENT_RESET:
            Log::add('R', 104);
            break;
        default:
            Log::add(errorCode.letter, errorCode.number);
        }
    }
}