#include <stdexcept>

#include "Gpu/Texture.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/Buffer.hpp"

namespace Anvil::Gpu
{
  Texture::Texture( const VulkanContext & context, const TextureDesc & desc,
                    std::span<const u8> pixels )
    : m_Context( context )
  {
    CreateImage( desc );
    UploadPixels( pixels, desc );
    CreateImageView( desc );
    CreateSampler();
  }

  Texture::~Texture()
  {
    const auto device = m_Context.GetDevice();

    if ( m_Sampler != VK_NULL_HANDLE )
    {
      vkDestroySampler( device, m_Sampler, nullptr );
    }

    if ( m_ImageView != VK_NULL_HANDLE )
    {
      vkDestroyImageView( device, m_ImageView, nullptr );
    }

    if ( m_Image != VK_NULL_HANDLE )
    {
      vkDestroyImage( device, m_Image, nullptr );
    }

    if ( m_Memory != VK_NULL_HANDLE )
    {
      vkFreeMemory( device, m_Memory, nullptr );
    }
  }

  void Texture::CreateImage( const TextureDesc & desc )
  {
    const auto device = m_Context.GetDevice();

    const VkImageCreateInfo imageInfo = {
      .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType   = VK_IMAGE_TYPE_2D,
      .format      = desc.m_Format,
      .extent      = { desc.m_Width, desc.m_Height, 1 },
      .mipLevels   = desc.m_MipLevels,
      .arrayLayers = 1,
      .samples     = VK_SAMPLE_COUNT_1_BIT,
      .tiling      = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE };

    if ( vkCreateImage( device, &imageInfo, nullptr, &m_Image ) != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create texture image." );
    }

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements( device, m_Image, &requirements );

    const u32 memoryType = m_Context.FindMemoryType(
      requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    const VkMemoryAllocateInfo allocInfo = {
      .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize  = requirements.size,
      .memoryTypeIndex = memoryType };

    if ( vkAllocateMemory( device, &allocInfo, nullptr, &m_Memory ) !=
         VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to allocate texture memory." );
    }

    vkBindImageMemory( device, m_Image, m_Memory, 0 );
  }

  void Texture::UploadPixels( std::span<const u8> pixels,
                              const TextureDesc & desc )
  {
    const auto dataSize = static_cast<VkDeviceSize>( pixels.size() );

    // Staging buffer lives on the stack.
    // RAII cleans it up after the copy.
    Buffer staging( m_Context, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    staging.Write( pixels.data(), dataSize );

    m_Context.SubmitOneShot(
      [ & ]( VkCommandBuffer cmd )
      {
        // Transition: undefined -> transfer destination.
        const VkImageMemoryBarrier2 toTransferDst = {
          .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
          .srcStageMask     = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
          .srcAccessMask    = 0,
          .dstStageMask     = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
          .dstAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
          .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
          .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          .image            = m_Image,
          .subresourceRange = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel   = 0,
                                .levelCount     = desc.m_MipLevels,
                                .baseArrayLayer = 0,
                                .layerCount     = 1 } };

        const VkDependencyInfo toTransferDep = {
          .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
          .imageMemoryBarrierCount = 1,
          .pImageMemoryBarriers    = &toTransferDst };

        vkCmdPipelineBarrier2( cmd, &toTransferDep );

        // Copy staging buffer to image.
        const VkBufferImageCopy2 region = {
          .sType             = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
          .bufferOffset      = 0,
          .bufferRowLength   = 0,
          .bufferImageHeight = 0,
          .imageSubresource  = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .mipLevel       = 0,
                                 .baseArrayLayer = 0,
                                 .layerCount     = 1 },
          .imageOffset       = { 0, 0, 0 },
          .imageExtent       = { desc.m_Width, desc.m_Height, 1 } };

        const VkCopyBufferToImageInfo2 copyInfo = {
          .sType          = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
          .srcBuffer      = staging.GetHandle(),
          .dstImage       = m_Image,
          .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          .regionCount    = 1,
          .pRegions       = &region };

        vkCmdCopyBufferToImage2( cmd, &copyInfo );

        // Transition: transfer destination -> shader read.
        const VkImageMemoryBarrier2 toShaderRead = {
          .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
          .srcStageMask     = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
          .srcAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
          .dstStageMask     = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
          .dstAccessMask    = VK_ACCESS_2_SHADER_READ_BIT,
          .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          .newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          .image            = m_Image,
          .subresourceRange = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel   = 0,
                                .levelCount     = desc.m_MipLevels,
                                .baseArrayLayer = 0,
                                .layerCount     = 1 } };

        const VkDependencyInfo toShaderDep = {
          .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
          .imageMemoryBarrierCount = 1,
          .pImageMemoryBarriers    = &toShaderRead };

        vkCmdPipelineBarrier2( cmd, &toShaderDep );
      } );
  }

  void Texture::CreateImageView( const TextureDesc & desc )
  {
    const VkImageViewCreateInfo viewInfo = {
      .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image            = m_Image,
      .viewType         = VK_IMAGE_VIEW_TYPE_2D,
      .format           = desc.m_Format,
      .components       = {},
      .subresourceRange = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel   = 0,
                            .levelCount     = desc.m_MipLevels,
                            .baseArrayLayer = 0,
                            .layerCount     = 1 } };

    if ( vkCreateImageView( m_Context.GetDevice(), &viewInfo, nullptr,
                            &m_ImageView ) != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create texture image view." );
    }
  }

  void Texture::CreateSampler()
  {
    const VkSamplerCreateInfo samplerInfo = {
      .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter    = VK_FILTER_LINEAR,
      .minFilter    = VK_FILTER_LINEAR,
      .mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .maxLod       = VK_LOD_CLAMP_NONE };

    if ( vkCreateSampler( m_Context.GetDevice(), &samplerInfo, nullptr,
                          &m_Sampler ) != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create texture sampler." );
    }
  }
} // namespace Anvil::Gpu
