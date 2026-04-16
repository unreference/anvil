#pragma once

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;
  class DescriptorAllocator;
  class Texture;

  class MaterialState
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( MaterialState );

    MaterialState( const VulkanContext & context,
                   DescriptorAllocator & allocator, const Texture & texture );
    ~MaterialState();

    [[nodiscard]] VkDescriptorSet GetDescriptorSet() const
    {
      return m_DescriptorSet;
    }

    [[nodiscard]] VkDescriptorSetLayout GetLayout() const
    {
      return m_Layout;
    }

  private:
    void CreateLayout();
    void CreateDescriptorSet( DescriptorAllocator & allocator,
                              const Texture &       texture );

    const VulkanContext & m_Context;
    VkDescriptorSetLayout m_Layout        = VK_NULL_HANDLE;
    VkDescriptorSet       m_DescriptorSet = VK_NULL_HANDLE;
  };
} // namespace Anvil::Gpu
