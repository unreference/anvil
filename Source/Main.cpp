#include <exception>
#include <iostream>
#include <chrono>

#include "Common/Version.hpp"
#include "Platform/Win32Window.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/SwapChain.hpp"
#include "Gpu/Renderer.hpp"
#include "Game/FreeCamera.hpp"

int main()
{
  try
  {
    using namespace Anvil;

#ifdef _DEBUG
    static constexpr const c16 * ConfigName = L"Debug";
#else
    static constexpr const c16 * ConfigName = L"Release";
#endif

    const std::wstring title =
      std::format( L"Toontown v{}.{}.{}.{} ({})", VersionMajor, VersionMinor,
                   VersionPatch, BuildNumber, ConfigName );

    Platform::Win32Window window( { .m_Title = title } );
    Gpu::VulkanContext    gpu( window.GetHandle(), window.GetInstance() );
    Gpu::SwapChain swapChain( gpu, window.GetWidth(), window.GetHeight() );
    Gpu::Renderer  renderer( gpu, swapChain );

    Game::FreeCamera camera( { 0.0f, 5.0f, 10.0f } );

    // Right-click to capture cursor and look around... eventually.
    window.SetCursorCaptured( false );

    auto previousTime = std::chrono::high_resolution_clock::now();

    while ( window.PollEvents() )
    {
      // Delta time.
      const auto currentTime = std::chrono::high_resolution_clock::now();
      const f32  deltaTime =
        std::chrono::duration<f32>( currentTime - previousTime ).count();
      previousTime = currentTime;

      // Toggle cursor capture with right mouse button.
      if ( window.IsKeyDown( VK_RBUTTON ) && !window.IsCursorCaptured() )
      {
        window.SetCursorCaptured( true );
      }

      if ( window.IsKeyDown( VK_ESCAPE ) && window.IsCursorCaptured() )
      {
        window.SetCursorCaptured( false );
      }

      // Build camera input from platform key state.
      const auto              mouseDelta  = window.GetMouseDelta();
      const Game::CameraInput cameraInput = {
        .m_IsMovingForward = window.IsKeyDown( 'W' ),
        .m_IsMovingBack    = window.IsKeyDown( 'S' ),
        .m_IsMovingLeft    = window.IsKeyDown( 'A' ),
        .m_IsMovingRight   = window.IsKeyDown( 'D' ),
        .m_IsMovingUp      = window.IsKeyDown( VK_SPACE ),
        .m_IsMovingDown    = window.IsKeyDown( VK_CONTROL ),
        .m_IsSprinting     = window.IsKeyDown( VK_SHIFT ),
        .m_MouseDeltaX     = mouseDelta.m_X,
        .m_MouseDeltaY     = mouseDelta.m_Y };

      camera.Update( deltaTime, cameraInput );
      window.ResetFrameInput();

#ifdef _DEBUG
      {
        const auto & pos = camera.GetPosition();
        const f32    fps = deltaTime > 0.0f ? 1.0f / deltaTime : 1.0f;

        window.SetTitle(
          std::format( L"{} | pos ({:.2f}, {:.2f}, {:.2f}) | {:.2f} fps", title,
                       pos.m_X, pos.m_Y, pos.m_Z, fps ) );
      }
#endif

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
