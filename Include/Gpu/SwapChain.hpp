#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;

  class SwapChain
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( SwapChain );

    SwapChain( const VulkanContext & context, u32 width, u32 height );
    ~SwapChain();

    void Recreate( u32 width, u32 height );

    [[nodiscard]] VkSwapchainKHR GetHandle() const
    {
      return m_SwapChain;
    }

    [[nodiscard]] VkFormat GetFormat() const
    {
      return m_Format;
    }

    [[nodiscard]] VkExtent2D GetExtent() const
    {
      return m_Extent;
    }

    [[nodiscard]] const std::vector<VkImageView> & GetImageViews() const
    {
      return m_ImageViews;
    }

    [[nodiscard]] const std::vector<VkImage> & GetImages() const
    {
      return m_Images;
    }

    [[nodiscard]] u32 GetImageCount() const
    {
      return static_cast<u32>( m_Images.size() );
    }

  private:
    void Create( u32 width, u32 height );
    void Destroy();

    const VulkanContext & m_Context;

    VkSwapchainKHR           m_SwapChain = VK_NULL_HANDLE;
    std::vector<VkImage>     m_Images;
    std::vector<VkImageView> m_ImageViews;
    VkFormat                 m_Format = VK_FORMAT_UNDEFINED;
    VkExtent2D               m_Extent = {};
  };
} // namespace Anvil::Gpu
