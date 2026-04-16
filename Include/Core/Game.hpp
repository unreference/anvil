#pragma once

#include <chrono>
#include <string>

#include "Common/Types.hpp"
#include "Platform/Win32Window.hpp"
#include "Gpu/VulkanContext.hpp"
#include "Gpu/SwapChain.hpp"
#include "Gpu/Renderer.hpp"
#include "Gpu/DescriptorAllocator.hpp"
#include "Gpu/FrameState.hpp"
#include "Gpu/Texture.hpp"
#include "Gpu/MaterialState.hpp"
#include "Gpu/Checkerboard.hpp"
#include "Gpu/Buffer.hpp"
#include "Gpu/StaticMeshPipeline.hpp"
#include "Gpu/Primitives.hpp"
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

    Gpu::DescriptorAllocator m_DescriptorAllocator;

    // Test checkerboard — lifetime matches Texture upload, not kept after.
    Gpu::CheckerboardResult m_CheckerboardData;
    Gpu::Texture            m_CheckerboardTexture;

    Gpu::FrameState    m_FrameState;
    Gpu::MaterialState m_MaterialState;

    // Cube test geometry — temporary until the asset pipeline lands.
    Gpu::Primitives::MeshData m_CubeMesh;
    Gpu::Buffer               m_VertexBuffer;
    Gpu::Buffer               m_IndexBuffer;
    Gpu::StaticMeshPipeline   m_Pipeline;

    Gameplay::FreeCamera  m_Camera;
    Gameplay::CameraInput m_CameraInput = {};

    f32 m_RotationAngle = 0.0f;

    std::chrono::high_resolution_clock::time_point m_PreviousTime {};
  };
} // namespace Anvil::Core