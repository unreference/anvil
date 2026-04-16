#pragma once

#include <span>
#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;

  class Buffer
  {
  public:
    ANVIL_NO_COPY( Buffer );

    Buffer( const VulkanContext & context, VkDeviceSize size,
            VkBufferUsageFlags usage, VkMemoryPropertyFlags properties );

    // Transfers ownership of the GPU resources.
    // The source is left in an empty state that destructs cleanly.
    Buffer( Buffer && other ) noexcept;

    ~Buffer();

    void Write( const void * data, VkDeviceSize size, VkDeviceSize offset = 0 );

    [[nodiscard]] VkBuffer GetHandle() const
    {
      return m_Buffer;
    }

    [[nodiscard]] VkDeviceSize GetSize() const
    {
      return m_Size;
    }

    [[nodiscard]] void * GetMappedData() const
    {
      return m_MappedData;
    }

    [[nodiscard]] bool IsMapped() const
    {
      return m_MappedData != nullptr;
    }

  private:
    const VulkanContext & m_Context;

    VkBuffer       m_Buffer     = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory     = VK_NULL_HANDLE;
    VkDeviceSize   m_Size       = 0;
    void *         m_MappedData = nullptr;
  };

  // Uploads data to a device-local buffer via a staging transfer.
  // The usage flags are OR'd with TRANSFER_DST internally.
  Buffer CreateDeviceLocalBuffer( const VulkanContext & context,
                                  std::span<const u8>   data,
                                  VkBufferUsageFlags    usage );
} // namespace Anvil::Gpu
