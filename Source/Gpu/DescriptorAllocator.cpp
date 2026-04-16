#include <stdexcept>

#include "Gpu/DescriptorAllocator.hpp"
#include "Gpu/VulkanContext.hpp"

namespace Anvil::Gpu
{
  DescriptorAllocator::DescriptorAllocator(
    const VulkanContext &                 context,
    std::span<const VkDescriptorPoolSize> poolSizes, u32 maxSets )
    : m_Context( context )
  {
    const VkDescriptorPoolCreateInfo poolInfo = {
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets       = maxSets,
      .poolSizeCount = static_cast<u32>( poolSizes.size() ),
      .pPoolSizes    = poolSizes.data() };

    if ( vkCreateDescriptorPool( m_Context.GetDevice(), &poolInfo, nullptr,
                                 &m_Pool ) != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create descriptor pool." );
    }
  }

  DescriptorAllocator::~DescriptorAllocator()
  {
    if ( m_Pool != VK_NULL_HANDLE )
    {
      vkDestroyDescriptorPool( m_Context.GetDevice(), m_Pool, nullptr );
    }
  }

  VkDescriptorSet DescriptorAllocator::Allocate( VkDescriptorSetLayout layout )
  {
    const VkDescriptorSetAllocateInfo allocInfo = {
      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool     = m_Pool,
      .descriptorSetCount = 1,
      .pSetLayouts        = &layout };

    VkDescriptorSet set = VK_NULL_HANDLE;

    if ( vkAllocateDescriptorSets( m_Context.GetDevice(), &allocInfo, &set ) !=
         VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to allocate descriptor set." );
    }

    return set;
  }
} // namespace Anvil::Gpu
