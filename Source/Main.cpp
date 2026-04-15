#include <exception>
#include <iostream>

#include "Common/Version.hpp"
#include "Platform/Win32Window.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/SwapChain.hpp"
#include "Gpu/Renderer.hpp"

int main()
{
  try
  {
#ifdef _DEBUG
    static constexpr const Anvil::c16 * ConfigName = L"Debug";
#else
    static constexpr const Anvil::c16 * ConfigName = L"Release";
#endif

    const std::wstring title = std::format(
      L"Toontown v{}.{}.{}.{} ({})", Anvil::VersionMajor, Anvil::VersionMinor,
      Anvil::VersionPatch, Anvil::BuildNumber, ConfigName );

    Anvil::Platform::Win32Window window( { .m_Title = title } );
    Anvil::Gpu::VulkanContext    gpu( window.GetHandle(), window.GetInstance() );
    Anvil::Gpu::SwapChain        swapChain( gpu, window.GetWidth(),
                                            window.GetHeight() );
    Anvil::Gpu::Renderer         renderer( gpu, swapChain );

    while ( window.PollEvents() )
    {
      if ( window.IsResized() )
      {
        swapChain.Recreate( window.GetWidth(), window.GetHeight() );
        renderer.OnSwapChainRecreated();
        window.ClearResizedFlag();
      }

      if ( !renderer.DrawFrame() )
      {
        swapChain.Recreate( window.GetWidth(), window.GetHeight() );
        renderer.OnSwapChainRecreated();
      }
    }

    vkDeviceWaitIdle( gpu.GetDevice() );
  }
  catch ( const std::exception & e )
  {
    std::cerr << std::format( "[Fatal] {}\n", e.what() );
    return 1;
  }

  return 0;
}
