#pragma once

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"
#include "Math/Mat4.hpp"
#include "Buffer.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;
  class DescriptorAllocator;

  class FrameState
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( FrameState );

    FrameState( const VulkanContext & context, DescriptorAllocator & allocator );
    ~FrameState();

    void Update( const Math::Mat4 & viewProjection );

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
    void CreateDescriptorSet( DescriptorAllocator & allocator );

    const VulkanContext & m_Context;

    Buffer                m_UniformBuffer;
    VkDescriptorSetLayout m_Layout        = VK_NULL_HANDLE;
    VkDescriptorSet       m_DescriptorSet = VK_NULL_HANDLE;
  };
} // namespace Anvil::Gpu
