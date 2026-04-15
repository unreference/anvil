#pragma once

#include <string>
#include <chrono>

#include "Common/Types.hpp"
#include "Platform/Win32Window.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/SwapChain.hpp"
#include "Gpu/Renderer.hpp"
#include "Gameplay/FreeCamera.hpp"

namespace Anvil::Core
{
  class Game
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( Game );

    Game();
    ~Game();

    void Run();

  private:
    void ProcessInput();
    void Update( f32 deltaTime );
    void Render();
    void OnResize();

#ifdef _DEBUG
    void UpdateDebugTitle( f32 deltaTime );
#endif

    std::wstring          m_BaseTitle;
    Platform::Win32Window m_Window;
    Gpu::VulkanContext    m_Gpu;
    Gpu::SwapChain        m_SwapChain;
    Gpu::Renderer         m_Renderer;

    Gameplay::FreeCamera  m_Camera;
    Gameplay::CameraInput m_CameraInput = {};

    std::chrono::high_resolution_clock::time_point m_PreviousTime{};
  };
} // namespace Anvil::Core
