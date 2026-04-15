#include <exception>
#include <iostream>

#include "Platform/Win32Window.hpp"

int main()
{
  try
  {
    Anvil::Platform::Win32Window window;
    std::cout << "Hello, world!" << std::endl;

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
