#include <stdexcept>

#include "Gpu/MaterialState.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/DescriptorAllocator.hpp"
#include "Gpu/Texture.hpp"

namespace Anvil::Gpu
{
  MaterialState::MaterialState( const VulkanContext & context,
                                DescriptorAllocator & allocator,
                                const Texture &       texture )
    : m_Context( context )
  {
    CreateLayout();
    CreateDescriptorSet( allocator, texture );
  }

  MaterialState::~MaterialState()
  {
    if ( m_Layout != VK_NULL_HANDLE )
    {
      vkDestroyDescriptorSetLayout( m_Context.GetDevice(), m_Layout, nullptr );
    }
  }

  void MaterialState::CreateLayout()
  {
    const VkDescriptorSetLayoutBinding binding = {
      .binding         = 0,
      .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT };

    const VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings    = &binding };

    if ( vkCreateDescriptorSetLayout( m_Context.GetDevice(), &layoutInfo,
                                      nullptr, &m_Layout ) != VK_SUCCESS )
    {
      throw std::runtime_error(
        "Failed to create material descriptor set layout." );
    }
  }

  void MaterialState::CreateDescriptorSet( DescriptorAllocator & allocator,
                                           const Texture &       texture )
  {
    m_DescriptorSet = allocator.Allocate( m_Layout );

    const VkDescriptorImageInfo imageInfo = {
      .sampler     = texture.GetSampler(),
      .imageView   = texture.GetImageView(),
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

    const VkWriteDescriptorSet write = {
      .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet          = m_DescriptorSet,
      .dstBinding      = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo      = &imageInfo };

    vkUpdateDescriptorSets( m_Context.GetDevice(), 1, &write, 0, nullptr );
  }
} // namespace Anvil::Gpu
