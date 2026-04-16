#pragma once

#include <span>

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;

  struct TextureDesc
  {
    u32      m_Width     = 0;
    u32      m_Height    = 0;
    u32      m_MipLevels = 1;
    VkFormat m_Format    = VK_FORMAT_R8G8B8A8_UNORM;
  };

  class Texture
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( Texture );

    Texture( const VulkanContext & context, const TextureDesc & desc,
             std::span<const u8> pixels );
    ~Texture();

    [[nodiscard]] VkImageView GetImageView() const
    {
      return m_ImageView;
    }

    [[nodiscard]] VkSampler GetSampler() const
    {
      return m_Sampler;
    }

  private:
    void CreateImage( const TextureDesc & desc );
    void UploadPixels( std::span<const u8> pixels, const TextureDesc & desc );
    void CreateImageView( const TextureDesc & desc );
    void CreateSampler();

    const VulkanContext & m_Context;

    VkImage        m_Image     = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory    = VK_NULL_HANDLE;
    VkImageView    m_ImageView = VK_NULL_HANDLE;
    VkSampler      m_Sampler   = VK_NULL_HANDLE;
  };
} // namespace Anvil::Gpu
