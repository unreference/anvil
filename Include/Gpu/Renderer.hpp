#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext;
  class SwapChain;

  static constexpr u32 MaxFramesInFlight = 2;

  struct FrameData
  {
    VkCommandBuffer m_CommandBuffer  = VK_NULL_HANDLE;
    VkSemaphore     m_ImageAvailable = VK_NULL_HANDLE;
    VkFence         m_InFlight       = VK_NULL_HANDLE;
  };

  class Renderer
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( Renderer );

    Renderer( const VulkanContext & context, const SwapChain & swapChain );
    ~Renderer();

    // Returns false if the swap chain needs recreation.
    bool DrawFrame();

    // Recreates per-image semaphores when the swap chain changes.
    void OnSwapChainRecreated();

  private:
    void CreateCommandPool();
    void CreateFrameData();
    void CreateImageSemaphores();
    void DestroyImageSemaphores();
    void RecordCommandBuffer( VkCommandBuffer cmd, VkImage image,
                              VkImageView view );

    const VulkanContext & m_Context;
    const SwapChain &     m_SwapChain;

    VkCommandPool m_CommandPool  = VK_NULL_HANDLE;
    u32           m_CurrentFrame = 0;

    std::array<FrameData, MaxFramesInFlight> m_Frames = {};

    // One per swap chain image to avoid semaphore reuse conflicts with the
    // presentation engine.
    std::vector<VkSemaphore> m_RenderFinished;
  };
} // namespace Anvil::Gpu
