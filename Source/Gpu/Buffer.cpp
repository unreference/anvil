#include <stdexcept>

#include "Gpu/Buffer.hpp"
#include "Gpu/VulkanContext.hpp"

namespace Anvil::Gpu
{
  Buffer::Buffer( const VulkanContext & context, VkDeviceSize size,
                  VkBufferUsageFlags usage, VkMemoryPropertyFlags properties )
    : m_Context( context )
    , m_Size( size )
  {
    const auto device = m_Context.GetDevice();

    const VkBufferCreateInfo bufferInfo = {
      .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size        = m_Size,
      .usage       = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE };

    if ( vkCreateBuffer( device, &bufferInfo, nullptr, &m_Buffer ) !=
         VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create buffer." );
    }

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements( device, m_Buffer, &requirements );

    const u32 memoryType =
      m_Context.FindMemoryType( requirements.memoryTypeBits, properties );

    const VkMemoryAllocateInfo allocInfo = {
      .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize  = requirements.size,
      .memoryTypeIndex = memoryType };

    if ( vkAllocateMemory( device, &allocInfo, nullptr, &m_Memory ) !=
         VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to allocate buffer memory." );
    }

    vkBindBufferMemory( device, m_Buffer, m_Memory, 0 );

    // Persistently map host-visible buffers.
    const bool isHostVisible =
      ( properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) != 0;

    if ( isHostVisible )
    {
      vkMapMemory( device, m_Memory, 0, m_Size, 0, &m_MappedData );
    }
  }

  Buffer::Buffer( Buffer && other ) noexcept
    : m_Context( other.m_Context )
    , m_Buffer( other.m_Buffer )
    , m_Memory( other.m_Memory )
    , m_Size( other.m_Size )
    , m_MappedData( other.m_MappedData )
  {
    other.m_Buffer     = VK_NULL_HANDLE;
    other.m_Memory     = VK_NULL_HANDLE;
    other.m_Size       = 0;
    other.m_MappedData = nullptr;
  }

  Buffer::~Buffer()
  {
    const auto device = m_Context.GetDevice();

    if ( m_MappedData )
    {
      vkUnmapMemory( device, m_Memory );
    }

    if ( m_Buffer != VK_NULL_HANDLE )
    {
      vkDestroyBuffer( device, m_Buffer, nullptr );
    }

    if ( m_Memory != VK_NULL_HANDLE )
    {
      vkFreeMemory( device, m_Memory, nullptr );
    }
  }

  void Buffer::Write( const void * data, VkDeviceSize size, VkDeviceSize offset )
  {
    if ( !m_MappedData )
    {
      throw std::runtime_error( "Cannot write to an unmapped buffer." );
    }

    std::memcpy( static_cast<u8 *>( m_MappedData ) + offset, data, size );
  }

  Buffer CreateDeviceLocalBuffer( const VulkanContext & context,
                                  std::span<const u8>   data,
                                  VkBufferUsageFlags    usage )
  {
    const VkDeviceSize size = data.size();

    Buffer staging( context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    staging.Write( data.data(), size );

    Buffer final( context, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    context.SubmitOneShot(
      [ & ]( VkCommandBuffer cmd )
      {
        const VkBufferCopy region = { .size = size };
        vkCmdCopyBuffer( cmd, staging.GetHandle(), final.GetHandle(), 1,
                         &region );
      } );

    return final;
  }
} // namespace Anvil::Gpu
