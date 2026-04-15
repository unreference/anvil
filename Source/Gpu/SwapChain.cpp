#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "Gpu/SwapChain.hpp"
#include "Gpu/VulkanContext.hpp"

namespace Anvil::Gpu
{
  SwapChain::SwapChain( const VulkanContext & context, u32 width, u32 height )
    : m_Context( context )
  {
    Create( width, height );
  }

  SwapChain::~SwapChain()
  {
    Destroy();
  }

  void SwapChain::Recreate( u32 width, u32 height )
  {
    vkDeviceWaitIdle( m_Context.GetDevice() );
    Destroy();
    Create( width, height );
  }

  void SwapChain::Create( u32 width, u32 height )
  {
    const auto device         = m_Context.GetDevice();
    const auto physicalDevice = m_Context.GetPhysicalDevice();
    const auto surface        = m_Context.GetSurface();

    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface,
                                               &capabilities );

    // Pick format - prefer B8G8R8A8_SRGB for correct gamma.
    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount,
                                          nullptr );

    std::vector<VkSurfaceFormatKHR> formats( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount,
                                          formats.data() );

    VkSurfaceFormatKHR selectedFormat = formats.at( 0 );
    for ( const auto & format : formats )
    {
      if ( format.format == VK_FORMAT_B8G8R8A8_SRGB &&
           format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
      {
        selectedFormat = format;
        break;
      }
    }

    m_Format = selectedFormat.format;

    // Clamp extent to surface capabilities.
    if ( capabilities.currentExtent.width != std::numeric_limits<u32>::max() )
    {
      m_Extent = capabilities.currentExtent;
    }
    else
    {
      m_Extent.width  = std::clamp( width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width );
      m_Extent.height = std::clamp( height, capabilities.minImageExtent.height,
                                    capabilities.maxImageExtent.height );
    }

    // Double buffer - request minimum + 1, capped by max.
    u32 imageCount = capabilities.minImageCount + 1;
    if ( capabilities.maxImageCount > 0 &&
         imageCount > capabilities.maxImageCount )
    {
      imageCount = capabilities.maxImageCount;
    }

    const VkSwapchainCreateInfoKHR createInfo = {
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface          = surface,
      .minImageCount    = imageCount,
      .imageFormat      = selectedFormat.format,
      .imageColorSpace  = selectedFormat.colorSpace,
      .imageExtent      = m_Extent,
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform     = capabilities.currentTransform,
      .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
      .clipped          = VK_TRUE };

    const VkResult result =
      vkCreateSwapchainKHR( device, &createInfo, nullptr, &m_SwapChain );

    if ( result != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create swap chain." );
    }

    // Retrieve the swap chain images.
    u32 actualCount = 0;
    vkGetSwapchainImagesKHR( device, m_SwapChain, &actualCount, nullptr );
    m_Images.resize( actualCount );
    vkGetSwapchainImagesKHR( device, m_SwapChain, &actualCount,
                             m_Images.data() );

    // Create one image view per swap chain image.
    m_ImageViews.resize( m_Images.size() );
    for ( usize i = 0; i < m_Images.size(); ++i )
    {
      const VkImageViewCreateInfo viewInfo = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = m_Images.at( i ),
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = m_Format,
        .components       = {},
        .subresourceRange = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                              .baseMipLevel   = 0,
                              .levelCount     = 1,
                              .baseArrayLayer = 0,
                              .layerCount     = 1 } };

      const VkResult viewResult =
        vkCreateImageView( device, &viewInfo, nullptr, &m_ImageViews.at( i ) );

      if ( viewResult != VK_SUCCESS )
      {
        throw std::runtime_error( "Failed to create swap chain image view." );
      }
    }

    std::cout << std::format( "[Anvil] Swap chain created ({}x{}, {} images).\n",
                              m_Extent.width, m_Extent.height, m_Images.size() );
  }

  void SwapChain::Destroy()
  {
    const auto device = m_Context.GetDevice();

    for ( auto view : m_ImageViews )
    {
      vkDestroyImageView( device, view, nullptr );
    }

    m_ImageViews.clear();
    m_Images.clear();

    if ( m_SwapChain != VK_NULL_HANDLE )
    {
      vkDestroySwapchainKHR( device, m_SwapChain, nullptr );
      m_SwapChain = VK_NULL_HANDLE;
    }
  }
} // namespace Anvil::Gpu
