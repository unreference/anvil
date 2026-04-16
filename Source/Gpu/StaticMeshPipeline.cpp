#include <array>
#include <stdexcept>

#include "Gpu/StaticMeshPipeline.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/StaticVertex.hpp"
#include "Math/Mat4.hpp"

namespace Anvil::Gpu
{
  StaticMeshPipeline::StaticMeshPipeline(
    const VulkanContext & context, const StaticMeshPipelineConfig & config )
    : m_Context( context )
  {
    const auto device = m_Context.GetDevice();

    // Shader modules are only needed during pipeline creation.
    VkShaderModule vertModule = CreateShaderModule( config.m_VertexShader );
    VkShaderModule fragModule = CreateShaderModule( config.m_FragmentShader );

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
      { { .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage  = VK_SHADER_STAGE_VERTEX_BIT,
          .module = vertModule,
          .pName  = "main" },
        { .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
          .module = fragModule,
          .pName  = "main" } } };

    const auto bindingDesc    = StaticVertex::GetBindingDescription();
    const auto attributeDescs = StaticVertex::GetAttributeDescriptions();

    const VkPipelineVertexInputStateCreateInfo vertexInput = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions    = &bindingDesc,
      .vertexAttributeDescriptionCount =
        static_cast<u32>( attributeDescs.size() ),
      .pVertexAttributeDescriptions = attributeDescs.data() };

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };

    const VkPipelineViewportStateCreateInfo viewportState = {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount  = 1 };

    const VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode    = VK_CULL_MODE_BACK_BIT,
      .frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .lineWidth   = 1.0f };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT };

    const VkPipelineDepthStencilStateCreateInfo depthStencil = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable  = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp   = VK_COMPARE_OP_LESS };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };

    const VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments    = &colorBlendAttachment };

    const std::array<VkDynamicState, 2> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    const VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<u32>( dynamicStates.size() ),
      .pDynamicStates    = dynamicStates.data() };

    // Push constant for per-draw model matrix.
    const VkPushConstantRange pushConstantRange = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .offset     = 0,
      .size       = sizeof( Math::Mat4 ) };

    const std::array<VkDescriptorSetLayout, 2> setLayouts = {
      config.m_SceneLayout, config.m_MaterialLayout };

    const VkPipelineLayoutCreateInfo layoutInfo = {
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount         = static_cast<u32>( setLayouts.size() ),
      .pSetLayouts            = setLayouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges    = &pushConstantRange };

    if ( vkCreatePipelineLayout( device, &layoutInfo, nullptr, &m_Layout ) !=
         VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create pipeline layout." );
    }

    // Dynamic rendering
    const VkPipelineRenderingCreateInfo renderingInfo = {
      .sType                = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &config.m_ColorFormat,
      .depthAttachmentFormat   = config.m_DepthFormat };

    const VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext               = &renderingInfo,
      .stageCount          = static_cast<u32>( shaderStages.size() ),
      .pStages             = shaderStages.data(),
      .pVertexInputState   = &vertexInput,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState      = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState   = &multisampling,
      .pDepthStencilState  = &depthStencil,
      .pColorBlendState    = &colorBlending,
      .pDynamicState       = &dynamicState,
      .layout              = m_Layout };

    if ( vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                    nullptr, &m_Pipeline ) != VK_SUCCESS )
    {
      vkDestroyShaderModule( device, fragModule, nullptr );
      vkDestroyShaderModule( device, vertModule, nullptr );
      throw std::runtime_error( "Failed to create graphics pipeline." );
    }

    vkDestroyShaderModule( device, fragModule, nullptr );
    vkDestroyShaderModule( device, vertModule, nullptr );
  }

  StaticMeshPipeline::~StaticMeshPipeline()
  {
    const auto device = m_Context.GetDevice();

    if ( m_Pipeline != VK_NULL_HANDLE )
    {
      vkDestroyPipeline( device, m_Pipeline, nullptr );
    }

    if ( m_Layout != VK_NULL_HANDLE )
    {
      vkDestroyPipelineLayout( device, m_Layout, nullptr );
    }
  }

  VkShaderModule
  StaticMeshPipeline::CreateShaderModule( std::span<const u32> spirV ) const
  {
    const VkShaderModuleCreateInfo createInfo = {
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = spirV.size_bytes(),
      .pCode    = spirV.data() };

    VkShaderModule module = VK_NULL_HANDLE;
    if ( vkCreateShaderModule( m_Context.GetDevice(), &createInfo, nullptr,
                               &module ) != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create shader module." );
    }

    return module;
  }
} // namespace Anvil::Gpu
