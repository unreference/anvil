#pragma once

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  class VulkanContext
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( VulkanContext );

    VulkanContext( HWND window, HINSTANCE instance );
    ~VulkanContext();

    [[nodiscard]] VkInstance GetInstance() const
    {
      return m_Instance;
    }

    [[nodiscard]] VkSurfaceKHR GetSurface() const
    {
      return m_Surface;
    }

    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const
    {
      return m_PhysicalDevice;
    }

    [[nodiscard]] VkDevice GetDevice() const
    {
      return m_Device;
    }

    [[nodiscard]] VkQueue GetQueue() const
    {
      return m_Queue;
    }

    [[nodiscard]] u32 GetQueueFamilyIndex() const
    {
      return m_QueueFamilyIndex;
    }

  private:
    void CreateInstance();
    void CreateDebugMessenger();
    void CreateSurface( HWND window, HINSTANCE instance );
    void SelectPhysicalDevice();
    void CreateDevice();

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT       severity,
      VkDebugUtilsMessageTypeFlagsEXT              types,
      const VkDebugUtilsMessengerCallbackDataEXT * data, void * userData );

    VkInstance               m_Instance         = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger   = VK_NULL_HANDLE;
    VkSurfaceKHR             m_Surface          = VK_NULL_HANDLE;
    VkPhysicalDevice         m_PhysicalDevice   = VK_NULL_HANDLE;
    VkDevice                 m_Device           = VK_NULL_HANDLE;
    VkQueue                  m_Queue            = VK_NULL_HANDLE;
    u32                      m_QueueFamilyIndex = 0;

#ifdef _DEBUG
    static constexpr bool IsValidationEnabled = true;
#else
    static constexpr bool IsValidationEnabled = false;
#endif // !_DEBUG
  };
} // namespace Anvil::Gpu
