#include <exception>
#include <iostream>

#include "Platform/Win32Window.hpp"
#include "Gpu/VulkanContext.hpp"

int main()
{
  try
  {
    Anvil::Platform::Win32Window window;
    Anvil::Gpu::VulkanContext    gpu( window.GetHandle(), window.GetInstance() );

    while ( window.PollEvents() )
    {
      if ( window.IsResized() )
      {
        // #TODO: Recreate swap chain.
        window.ClearResizedFlag();
      }

      // #TODO: Vulkan frame submission.
    }
  }
  catch ( const std::exception & e )
  {
    std::cerr << "[Fatal] " << e.what() << '\n';
    return 1;
  }

  return 0;
}
