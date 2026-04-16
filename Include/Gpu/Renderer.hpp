#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"
#include "Math/Mat4.hpp"

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

  struct DrawParams
  {
    VkPipeline       m_Pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet  m_SceneSet       = VK_NULL_HANDLE;
    VkDescriptorSet  m_MaterialSet    = VK_NULL_HANDLE;
    VkBuffer         m_VertexBuffer   = VK_NULL_HANDLE;
    VkBuffer         m_IndexBuffer    = VK_NULL_HANDLE;
    u32              m_IndexCount     = 0;
    Math::Mat4       m_ModelMatrix    = Math::Mat4::Identity();
  };

  class Renderer
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( Renderer );

    Renderer( const VulkanContext & context, const SwapChain & swapChain );
    ~Renderer();

    // Returns false if the swap chain needs recreation.
    bool DrawFrame( const DrawParams & params );

    // Recreates per-image semaphores when the swap chain changes.
    void OnSwapChainRecreated();

  private:
    void CreateCommandPool();
    void CreateFrameData();
    void CreateImageSemaphores();
    void DestroyImageSemaphores();
    void CreateDepthResources();
    void DestroyDepthResources();
    void RecordCommandBuffer( VkCommandBuffer cmd, VkImage image,
                              VkImageView view, const DrawParams & params );

    const VulkanContext & m_Context;
    const SwapChain &     m_SwapChain;

    VkCommandPool m_CommandPool  = VK_NULL_HANDLE;
    u32           m_CurrentFrame = 0;

    std::array<FrameData, MaxFramesInFlight> m_Frames = {};

    // One per swap chain image to avoid semaphore reuse conflicts with the
    // presentation engine.
    std::vector<VkSemaphore> m_RenderFinished;

    // Depth buffer matches swap chain extent, recreated alongside it.
    VkImage        m_DepthImage  = VK_NULL_HANDLE;
    VkDeviceMemory m_DepthMemory = VK_NULL_HANDLE;
    VkImageView    m_DepthView   = VK_NULL_HANDLE;

    static constexpr VkFormat DepthFormat = VK_FORMAT_D32_SFLOAT;
  };
} // namespace Anvil::Gpu
