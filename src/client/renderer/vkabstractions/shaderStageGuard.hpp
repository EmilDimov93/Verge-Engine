// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../tools.hpp"

#include "../../../shared/log.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>

namespace VE
{
    class ShaderStageGuard
    {
    public:
        ShaderStageGuard(std::string path, VkShaderStageFlagBits stageFlag, VkDevice device) : device(device)
        {
            std::vector<char> code = readFile(path);

            if (code.empty())
                Log::add('R', 221);

            VkShaderModuleCreateInfo shaderModuleCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t *>(code.data())};

            VkResult res = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module);
            if (res != VK_SUCCESS)
                Log::add('R', 209);

            stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage.stage = stageFlag;
            stage.module = module;
            stage.pName = "main";
        };

        ~ShaderStageGuard()
        {
            vkDestroyShaderModule(device, module, nullptr);
        };

        operator VkPipelineShaderStageCreateInfo() { return stage; };
        const VkPipelineShaderStageCreateInfo *operator&() const { return &stage; }

    private:
        VkPipelineShaderStageCreateInfo stage = {};
        VkShaderModule module = VK_NULL_HANDLE;

        VkDevice device = VK_NULL_HANDLE;
    };
}