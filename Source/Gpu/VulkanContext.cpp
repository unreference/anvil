#include <vector>
#include <iostream>
#include <format>
#include <array>

#include "Gpu/VulkanContext.hpp"

namespace Anvil::Gpu
{
  VulkanContext::VulkanContext( HWND window, HINSTANCE instance )
  {
    CreateInstance();
    CreateDebugMessenger();
    CreateSurface( window, instance );
    SelectPhysicalDevice();
    CreateDevice();
  };

  VulkanContext::~VulkanContext()
  {
    if ( m_Device != VK_NULL_HANDLE )
    {
      vkDeviceWaitIdle( m_Device );
      vkDestroyDevice( m_Device, nullptr );
    }

    if ( m_Surface != VK_NULL_HANDLE )
    {
      vkDestroySurfaceKHR( m_Instance, m_Surface, nullptr );
    }

    if ( m_DebugMessenger != VK_NULL_HANDLE )
    {
      auto Destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr( m_Instance, "vkDestroyDebugUtilsMessengerEXT" ) );

      if ( Destroy )
      {
        Destroy( m_Instance, m_DebugMessenger, nullptr );
      }
    }

    if ( m_Instance != VK_NULL_HANDLE )
    {
      vkDestroyInstance( m_Instance, nullptr );
    }
  }

  void VulkanContext::CreateInstance()
  {
    const VkApplicationInfo appInfo = {
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName   = "Toontown",
      .applicationVersion = VK_MAKE_VERSION( 0, 1, 0 ),
      .pEngineName        = "Anvil",
      .engineVersion      = VK_MAKE_VERSION( 0, 1, 0 ),
      .apiVersion         = VK_API_VERSION_1_3 };

    // Required extensions for windowed presentation.
    std::vector<const c8 *> extensions = { VK_KHR_SURFACE_EXTENSION_NAME,
                                           VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

    // Validation layers for debug builds.
    std::vector<const c8 *> layers;

    if constexpr ( IsValidationEnabled )
    {
      extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
      layers.push_back( "VK_LAYER_KHRONOS_validation" );
      std::cout << "[Anvil] Validation layers enabled.\n";
    }

    const VkInstanceCreateInfo createInfo = {
      .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo        = &appInfo,
      .enabledLayerCount       = static_cast<u32>( layers.size() ),
      .ppEnabledLayerNames     = layers.data(),
      .enabledExtensionCount   = static_cast<u32>( extensions.size() ),
      .ppEnabledExtensionNames = extensions.data() };

    const VkResult result =
      vkCreateInstance( &createInfo, nullptr, &m_Instance );

    if ( result != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create Vulkan instance." );
    }

    std::cout << "[Anvil] Vulkan instance created.\n";
  }

  void VulkanContext::CreateDebugMessenger()
  {
    if constexpr ( IsValidationEnabled )
    {
      const VkDebugUtilsMessengerCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugCallback };

      auto Create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr( m_Instance, "vkCreateDebugUtilsMessengerEXT" ) );

      if ( !Create )
      {
        std::cerr << "[Anvil] Debug messenger extension not available.\n";
        return;
      }

      const VkResult result =
        Create( m_Instance, &createInfo, nullptr, &m_DebugMessenger );

      if ( result != VK_SUCCESS )
      {
        std::cerr << "[Anvil] Failed to create debug messenger.\n";
        return;
      }

      std::cout << "[Anvil] Debug messenger created.\n";
    }

    return;
  }

  VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT * data, void * )
  {
    const c8 * level = "INFO";

    if ( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
    {
      level = "ERROR";
    }
    else if ( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
    {
      level = "WARN";
    }

    std::cerr << std::format( "[Vulkan {}] {}\n", level, data->pMessage );
    return VK_FALSE;
  }

  void VulkanContext::CreateSurface( HWND window, HINSTANCE instance )
  {
    const VkWin32SurfaceCreateInfoKHR createInfo = {
      .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = instance,
      .hwnd      = window };

    const VkResult result =
      vkCreateWin32SurfaceKHR( m_Instance, &createInfo, nullptr, &m_Surface );

    if ( result != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create Win32 surface." );
    }

    std::cout << "[Anvil] Win32 surface created.\n";
  }

  void VulkanContext::SelectPhysicalDevice()
  {
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices( m_Instance, &deviceCount, nullptr );

    if ( deviceCount == 0 )
    {
      throw std::runtime_error( "No Vulkan-capable GPUs found." );
    }

    std::vector<VkPhysicalDevice> devices( deviceCount );
    vkEnumeratePhysicalDevices( m_Instance, &deviceCount, devices.data() );

    for ( const auto & device : devices )
    {
      // Check API version.
      VkPhysicalDeviceProperties properties = {};
      vkGetPhysicalDeviceProperties( device, &properties );

      if ( properties.apiVersion < VK_API_VERSION_1_3 )
      {
        continue;
      }

      // Check for dynamic rendering support.
      VkPhysicalDeviceVulkan13Features features13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };

      VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features13 };

      vkGetPhysicalDeviceFeatures2( device, &features2 );

      if ( !features13.dynamicRendering || !features13.synchronization2 )
      {
        continue;
      }

      // Check for swap chain extension support.
      u32 extensionCount = 0;
      vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount,
                                            nullptr );

      std::vector<VkExtensionProperties> extensions( extensionCount );
      vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount,
                                            extensions.data() );

      bool isSwapChainSupported = false;
      for ( const auto & extension : extensions )
      {
        if ( std::string_view( extension.extensionName ) ==
             VK_KHR_SWAPCHAIN_EXTENSION_NAME )
        {
          isSwapChainSupported = true;
          break;
        }
      }

      if ( !isSwapChainSupported )
      {
        continue;
      }

      // Find a queue family with graphics + present support.
      u32 queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount,
                                                nullptr );

      std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
      vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount,
                                                queueFamilies.data() );

      for ( u32 i = 0; i < queueFamilyCount; ++i )
      {
        const bool isGraphicsSupported =
          queueFamilies.at( i ).queueFlags & VK_QUEUE_GRAPHICS_BIT;

        VkBool32 isPresentSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, m_Surface,
                                              &isPresentSupported );

        if ( isGraphicsSupported && isPresentSupported )
        {
          m_PhysicalDevice   = device;
          m_QueueFamilyIndex = i;

          std::cout << std::format(
            "[Anvil] Selected GPU: {} (queue family {}).\n",
            properties.deviceName, i );
          return;
        }
      }
    }

    throw std::runtime_error( "No suitable GPU found." );
  }

  void VulkanContext::CreateDevice()
  {
    const f32 queuePriority = 1.0f;

    const VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = m_QueueFamilyIndex,
      .queueCount       = 1,
      .pQueuePriorities = &queuePriority };

    // Enable the 1.3 features we selected the device for.
    VkPhysicalDeviceVulkan13Features features13 = {
      .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .synchronization2 = VK_TRUE,
      .dynamicRendering = VK_TRUE };

    VkPhysicalDeviceFeatures2 features2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &features13 };

    const std::array<const c8 *, 1> extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    const VkDeviceCreateInfo createInfo = {
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext                   = &features2,
      .queueCreateInfoCount    = 1,
      .pQueueCreateInfos       = &queueCreateInfo,
      .enabledExtensionCount   = static_cast<u32>( extensions.size() ),
      .ppEnabledExtensionNames = extensions.data() };

    const VkResult result =
      vkCreateDevice( m_PhysicalDevice, &createInfo, nullptr, &m_Device );

    if ( result != VK_SUCCESS )
    {
      throw std::runtime_error( "Failed to create logical device." );
    }

    vkGetDeviceQueue( m_Device, m_QueueFamilyIndex, 0, &m_Queue );
    std::cout << "[Anvil] Logical device and queue created.\n";
  }
} // namespace Anvil::Gpu
