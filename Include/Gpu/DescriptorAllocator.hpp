#pragma once

#include <span>

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;

  class DescriptorAllocator
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( DescriptorAllocator );

    DescriptorAllocator( const VulkanContext &                 context,
                         std::span<const VkDescriptorPoolSize> poolSizes,
                         u32                                   maxSets );
    ~DescriptorAllocator();

    [[nodiscard]] VkDescriptorSet Allocate( VkDescriptorSetLayout layout );

  private:
    const VulkanContext & m_Context;
    VkDescriptorPool      m_Pool = VK_NULL_HANDLE;
  };
} // namespace Anvil::Gpu
