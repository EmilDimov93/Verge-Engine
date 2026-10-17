// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../../shared/drawData.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <array>

namespace VE
{
    constexpr VkVertexInputBindingDescription DEFAULT_BINDING_DESCRIPTION = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    constexpr VkPipelineInputAssemblyStateCreateInfo DEFAULT_INPUT_ASSEMBLY_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    constexpr VkPipelineMultisampleStateCreateInfo DEFAULT_MULTISAMPLE_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE};

    constexpr std::array<VkDynamicState, 2> DEFAULT_DYNAMIC_STATE = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    constexpr VkPipelineDynamicStateCreateInfo DEFAULT_DYNAMIC_STATE_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = DEFAULT_DYNAMIC_STATE.data()};

    constexpr VkPipelineViewportStateCreateInfo DEFAULT_VIEWPORT_STATE_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1};

    struct GraphicsPipeline
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;
    };
}