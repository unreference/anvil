#include <iostream>

#include "Gpu/Renderer.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/SwapChain.hpp"

namespace Anvil::Gpu
{

  Renderer::Renderer( const VulkanContext & context,
                      const SwapChain &     swapChain )
    : m_Context( context )
    , m_SwapChain( swapChain )
  {
    CreateCommandPool();
    CreateFrameData();
    CreateImageSemaphores();
  }

  Renderer::~Renderer()
  {
    const auto device = m_Context.GetDevice();

    DestroyImageSemaphores();

    for ( auto & frame : m_Frames )
    {
      vkDestroySemaphore( device, frame.m_ImageAvailable, nullptr );
      vkDestroyFence( device, frame.m_InFlight, nullptr );
    }

    // Command buffers are freed implicitly when the pool is destroyed.
    vkDestroyCommandPool( device, m_CommandPool, nullptr );
    std::cout << "[Anvil] Renderer destroyed.\n";
  }

  void Renderer::CreateCommandPool()
  {
    const VkCommandPoolCreateInfo createInfo = {
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      // Allows individual command buffers to be reset and re-recorded.
      .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = m_Context.GetQueueFamilyIndex() };

    const VkResult result = vkCreateCommandPool(
      m_Context.GetDevice(), &createInfo, nullptr, &m_CommandPool );

    if ( result != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create command pool." );
    }

    std::cout << "[Anvil] Command pool created.\n";
  }

  void Renderer::CreateFrameData()
  {
    const auto device = m_Context.GetDevice();

    const VkCommandBufferAllocateInfo allocInfo = {
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = m_CommandPool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = MaxFramesInFlight };

    // Allocate all command buffers in one call.
    std::array<VkCommandBuffer, MaxFramesInFlight> commandBuffers = {};
    const VkResult                                 allocResult =
      vkAllocateCommandBuffers( device, &allocInfo, commandBuffers.data() );

    if ( allocResult != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to allocate command buffers." );
    }

    const VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    const VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      // Start signaled so the first frame doesn't deadlock on wait.
      .flags = VK_FENCE_CREATE_SIGNALED_BIT };

    for ( usize i = 0; i < MaxFramesInFlight; ++i )
    {
      auto & frame          = m_Frames.at( i );
      frame.m_CommandBuffer = commandBuffers.at( i );

      if ( vkCreateSemaphore( device, &semaphoreInfo, nullptr,
                              &frame.m_ImageAvailable ) != VK_SUCCESS ||
           vkCreateFence( device, &fenceInfo, nullptr, &frame.m_InFlight ) !=
             VK_SUCCESS )
      {
        throw std::runtime_error( "Failed to create frame sync objects." );
      }
    }

    std::cout << "[Anvil] Frame resources created.\n";
  }

  bool Renderer::DrawFrame()
  {
    const auto device = m_Context.GetDevice();
    const auto queue  = m_Context.GetQueue();
    auto &     frame  = m_Frames.at( m_CurrentFrame );

    vkWaitForFences( device, 1, &frame.m_InFlight, VK_TRUE,
                     std::numeric_limits<u64>::max() );

    u32      imageIndex    = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(
      device, m_SwapChain.GetHandle(), std::numeric_limits<u64>::max(),
      frame.m_ImageAvailable, VK_NULL_HANDLE, &imageIndex );

    if ( acquireResult == VK_ERROR_OUT_OF_DATE_KHR )
    {
      return false;
    }

    vkResetFences( device, 1, &frame.m_InFlight );
    vkResetCommandBuffer( frame.m_CommandBuffer, 0 );

    RecordCommandBuffer( frame.m_CommandBuffer,
                         m_SwapChain.GetImages().at( imageIndex ),
                         m_SwapChain.GetImageViews().at( imageIndex ) );

    // Index render-finished semaphore by swap chain image to avoid reuse while
    // the presentation engine holds it.
    VkSemaphore renderFinished = m_RenderFinished.at( imageIndex );

    const VkPipelineStageFlags waitStage =
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                      .waitSemaphoreCount = 1,
                                      .pWaitSemaphores = &frame.m_ImageAvailable,
                                      .pWaitDstStageMask  = &waitStage,
                                      .commandBufferCount = 1,
                                      .pCommandBuffers = &frame.m_CommandBuffer,
                                      .signalSemaphoreCount = 1,
                                      .pSignalSemaphores    = &renderFinished };

    if ( vkQueueSubmit( queue, 1, &submitInfo, frame.m_InFlight ) != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to submit draw command buffer." );
    }

    const VkSwapchainKHR swapChainHandle = m_SwapChain.GetHandle();

    const VkPresentInfoKHR presentInfo = { .sType =
                                             VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                           .waitSemaphoreCount = 1,
                                           .pWaitSemaphores    = &renderFinished,
                                           .swapchainCount     = 1,
                                           .pSwapchains   = &swapChainHandle,
                                           .pImageIndices = &imageIndex };

    VkResult presentResult = vkQueuePresentKHR( queue, &presentInfo );

    if ( presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
         presentResult == VK_SUBOPTIMAL_KHR )
    {
      return false;
    }

    m_CurrentFrame = ( m_CurrentFrame + 1 ) % MaxFramesInFlight;
    return true;
  }

  void Renderer::RecordCommandBuffer( VkCommandBuffer cmd, VkImage image,
                                      VkImageView view )
  {
    const VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };

    vkBeginCommandBuffer( cmd, &beginInfo );

    // Transition from whatever state to color attachment.
    const VkImageMemoryBarrier2 toColorAttachment = {
      .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask     = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
      .srcAccessMask    = 0,
      .dstStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image            = image,
      .subresourceRange = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel   = 0,
                            .levelCount     = 1,
                            .baseArrayLayer = 0,
                            .layerCount     = 1 } };

    const VkDependencyInfo toColorDep = {
      .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers    = &toColorAttachment };

    vkCmdPipelineBarrier2( cmd, &toColorDep );

    const VkRenderingAttachmentInfo colorAttachment = {
      .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView   = view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue  = { .color = { { 0.36f, 0.67f, 0.93f, 1.0f } } } };

    const VkRenderingInfo renderingInfo = {
      .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea           = { { 0, 0 }, m_SwapChain.GetExtent() },
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &colorAttachment };

    vkCmdBeginRendering( cmd, &renderingInfo );
    vkCmdEndRendering( cmd );

    // Transition from color attachment to presentable.
    const VkImageMemoryBarrier2 toPresent = {
      .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .dstStageMask     = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
      .dstAccessMask    = 0,
      .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .image            = image,
      .subresourceRange = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel   = 0,
                            .levelCount     = 1,
                            .baseArrayLayer = 0,
                            .layerCount     = 1 } };

    const VkDependencyInfo toPresentDep = { .sType =
                                              VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                            .imageMemoryBarrierCount = 1,
                                            .pImageMemoryBarriers = &toPresent };

    vkCmdPipelineBarrier2( cmd, &toPresentDep );
    vkEndCommandBuffer( cmd );
  }

  void Renderer::OnSwapChainRecreated()
  {
    DestroyImageSemaphores();
    CreateImageSemaphores();
  }

  void Renderer::CreateImageSemaphores()
  {
    const auto device     = m_Context.GetDevice();
    const u32  imageCount = m_SwapChain.GetImageCount();

    const VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    m_RenderFinished.resize( imageCount );
    for ( usize i = 0; i < imageCount; ++i )
    {
      if ( vkCreateSemaphore( device, &semaphoreInfo, nullptr,
                              &m_RenderFinished.at( i ) ) != VK_SUCCESS )
      {
        throw std::runtime_error(
          "Failed to create per-image render semaphore." );
      }
    }
  }

  void Renderer::DestroyImageSemaphores()
  {
    const auto device = m_Context.GetDevice();

    for ( auto semaphore : m_RenderFinished )
    {
      vkDestroySemaphore( device, semaphore, nullptr );
    }

    m_RenderFinished.clear();
  }
} // namespace Anvil::Gpu
