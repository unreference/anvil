#include <stdexcept>

#include "Gpu/FrameState.hpp"
#include "Gpu/DescriptorAllocator.hpp"
#include "Gpu/VulkanContext.hpp"

namespace Anvil::Gpu
{
  FrameState::FrameState( const VulkanContext & context,
                          DescriptorAllocator & allocator )
    : m_Context( context )
    , m_UniformBuffer( context, sizeof( Math::Mat4 ),
                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
  {
    CreateLayout();
    CreateDescriptorSet( allocator );
  }

  FrameState::~FrameState()
  {
    if ( m_Layout != VK_NULL_HANDLE )
    {
      vkDestroyDescriptorSetLayout( m_Context.GetDevice(), m_Layout, nullptr );
    }
  }

  void FrameState::Update( const Math::Mat4 & viewProjection )
  {
    m_UniformBuffer.Write( viewProjection.Data(), sizeof( Math::Mat4 ) );
  }

  void FrameState::CreateLayout()
  {
    const VkDescriptorSetLayoutBinding binding = {
      .binding         = 0,
      .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT };

    const VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings    = &binding };

    if ( vkCreateDescriptorSetLayout( m_Context.GetDevice(), &layoutInfo,
                                      nullptr, &m_Layout ) != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create descriptor set layout." );
    }
  }

  void FrameState::CreateDescriptorSet( DescriptorAllocator & allocator )
  {
    m_DescriptorSet = allocator.Allocate( m_Layout );

    const VkDescriptorBufferInfo bufferInfo = { .buffer =
                                                  m_UniformBuffer.GetHandle(),
                                                .offset = 0,
                                                .range  = sizeof( Math::Mat4 ) };

    const VkWriteDescriptorSet write = {
      .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet          = m_DescriptorSet,
      .dstBinding      = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo     = &bufferInfo };

    vkUpdateDescriptorSets( m_Context.GetDevice(), 1, &write, 0, nullptr );
  }
} // namespace Anvil::Gpu
