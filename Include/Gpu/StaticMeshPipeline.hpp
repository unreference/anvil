#pragma once

#include <span>

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;

  struct StaticMeshPipelineConfig
  {
    std::span<const u32>  m_VertexShader;
    std::span<const u32>  m_FragmentShader;
    VkDescriptorSetLayout m_SceneLayout    = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_MaterialLayout = VK_NULL_HANDLE;
    VkFormat              m_ColorFormat    = VK_FORMAT_UNDEFINED;
    VkFormat              m_DepthFormat    = VK_FORMAT_D32_SFLOAT;
  };

  class StaticMeshPipeline
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( StaticMeshPipeline );

    StaticMeshPipeline( const VulkanContext &            context,
                        const StaticMeshPipelineConfig & config );
    ~StaticMeshPipeline();

    [[nodiscard]] VkPipeline GetHandle() const
    {
      return m_Pipeline;
    }

    [[nodiscard]] VkPipelineLayout GetLayout() const
    {
      return m_Layout;
    }

  private:
    VkShaderModule CreateShaderModule( std::span<const u32> spirV ) const;

    const VulkanContext & m_Context;
    VkPipelineLayout      m_Layout   = VK_NULL_HANDLE;
    VkPipeline            m_Pipeline = VK_NULL_HANDLE;
  };
} // namespace Anvil::Gpu
