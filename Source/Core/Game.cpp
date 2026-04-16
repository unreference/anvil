#include <array>
#include <chrono>
#include <format>
#include <thread>

#include "Core/Game.hpp"
#include "Common/Version.hpp"
#include "Math/Transforms.hpp"
#include "Resources/Shaders/BasicVert.hpp"
#include "Resources/Shaders/BasicFrag.hpp"

namespace Anvil::Core
{
  // One UBO descriptor for FrameState, one combined image
  // sampler for MaterialState. Grow as more sets are needed.
  static constexpr std::array<VkDescriptorPoolSize, 2> DescriptorPoolSizes = { {
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
  } };

  static constexpr u32 MaxDescriptorSets = 2;

  static std::wstring BuildTitle()
  {
#ifdef _DEBUG
    static constexpr const c16 * ConfigName = L"Debug";
#else
    static constexpr const c16 * ConfigName = L"Release";
#endif

    return std::format( L"Toontown v{}.{}.{}.{} ({})", VersionMajor,
                        VersionMinor, VersionPatch, BuildNumber, ConfigName );
  }

  static std::span<const u8>
  VertexBytes( const Gpu::Primitives::MeshData & mesh )
  {
    return { reinterpret_cast<const u8 *>( mesh.m_Vertices.data() ),
             mesh.m_Vertices.size() * sizeof( Gpu::StaticVertex ) };
  }

  static std::span<const u8> IndexBytes( const Gpu::Primitives::MeshData & mesh )
  {
    return { reinterpret_cast<const u8 *>( mesh.m_Indices.data() ),
             mesh.m_Indices.size() * sizeof( u16 ) };
  }

  Game::Game()
    : m_BaseTitle( BuildTitle() )
    , m_Window( { .m_Title = m_BaseTitle } )
    , m_Gpu( m_Window.GetHandle(), m_Window.GetInstance() )
    , m_SwapChain( m_Gpu, m_Window.GetWidth(), m_Window.GetHeight() )
    , m_Renderer( m_Gpu, m_SwapChain )
    , m_DescriptorAllocator( m_Gpu, DescriptorPoolSizes, MaxDescriptorSets )
    , m_CheckerboardData( Gpu::GenerateCheckerboard() )
    , m_CheckerboardTexture( m_Gpu, m_CheckerboardData.m_Desc,
                             m_CheckerboardData.m_Pixels )
    , m_FrameState( m_Gpu, m_DescriptorAllocator )
    , m_MaterialState( m_Gpu, m_DescriptorAllocator, m_CheckerboardTexture )
    , m_CubeMesh( Gpu::Primitives::GenerateCube() )
    , m_VertexBuffer( Gpu::CreateDeviceLocalBuffer(
        m_Gpu, VertexBytes( m_CubeMesh ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) )
    , m_IndexBuffer( Gpu::CreateDeviceLocalBuffer(
        m_Gpu, IndexBytes( m_CubeMesh ), VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) )
    , m_Pipeline( m_Gpu,
                  Gpu::StaticMeshPipelineConfig {
                    .m_VertexShader   = Shaders::BasicVert,
                    .m_FragmentShader = Shaders::BasicFrag,
                    .m_SceneLayout    = m_FrameState.GetLayout(),
                    .m_MaterialLayout = m_MaterialState.GetLayout(),
                    .m_ColorFormat    = m_SwapChain.GetFormat(),
                    .m_DepthFormat    = VK_FORMAT_D32_SFLOAT,
                  } )
    , m_Camera( { 0.0f, 5.0f, 10.0f } )
    , m_PreviousTime( std::chrono::high_resolution_clock::now() )
  {
  }

  Game::~Game()
  {
    vkDeviceWaitIdle( m_Gpu.GetDevice() );
  }

  void Game::Run()
  {
    while ( m_Window.PollEvents() )
    {
      const auto currentTime = std::chrono::high_resolution_clock::now();
      const f32  deltaTime =
        std::chrono::duration<f32>( currentTime - m_PreviousTime ).count();
      m_PreviousTime = currentTime;

      ProcessInput();
      Update( deltaTime );
      Render();

#ifdef _DEBUG
      UpdateDebugTitle( deltaTime );
#endif
    }
  }

  void Game::ProcessInput()
  {
    if ( m_Window.IsKeyDown( VK_RBUTTON ) && !m_Window.IsCursorCaptured() )
    {
      m_Window.SetCursorCaptured( true );
    }

    if ( m_Window.IsKeyDown( VK_ESCAPE ) && m_Window.IsCursorCaptured() )
    {
      m_Window.SetCursorCaptured( false );
    }

    const auto mouseDelta = m_Window.GetMouseDelta();

    m_CameraInput = { .m_IsMovingForward = m_Window.IsKeyDown( 'W' ),
                      .m_IsMovingBack    = m_Window.IsKeyDown( 'S' ),
                      .m_IsMovingLeft    = m_Window.IsKeyDown( 'A' ),
                      .m_IsMovingRight   = m_Window.IsKeyDown( 'D' ),
                      .m_IsMovingUp      = m_Window.IsKeyDown( VK_SPACE ),
                      .m_IsMovingDown    = m_Window.IsKeyDown( VK_CONTROL ),
                      .m_IsSprinting     = m_Window.IsKeyDown( VK_SHIFT ),
                      .m_MouseDeltaX     = mouseDelta.m_X,
                      .m_MouseDeltaY     = mouseDelta.m_Y };

    m_Window.ResetFrameInput();
  }

  void Game::Update( f32 deltaTime )
  {
    m_Camera.Update( deltaTime, m_CameraInput );

    m_RotationAngle += deltaTime;

    const f32 aspect = static_cast<f32>( m_Window.GetWidth() ) /
                       static_cast<f32>( m_Window.GetHeight() );

    const Math::Mat4 view       = m_Camera.GetViewMatrix();
    const Math::Mat4 projection = m_Camera.GetProjectionMatrix( aspect );

    // Pre-multiply on CPU.  One matrix to the GPU instead of two.
    m_FrameState.Update( projection * view );
  }

  void Game::Render()
  {
    if ( m_Window.IsMinimized() )
    {
      std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
      return;
    }

    if ( m_Window.IsResized() )
    {
      OnResize();
      m_Window.ClearResizedFlag();
    }

    const Gpu::DrawParams params = {
      .m_Pipeline       = m_Pipeline.GetHandle(),
      .m_PipelineLayout = m_Pipeline.GetLayout(),
      .m_SceneSet       = m_FrameState.GetDescriptorSet(),
      .m_MaterialSet    = m_MaterialState.GetDescriptorSet(),
      .m_VertexBuffer   = m_VertexBuffer.GetHandle(),
      .m_IndexBuffer    = m_IndexBuffer.GetHandle(),
      .m_IndexCount     = static_cast<u32>( m_CubeMesh.m_Indices.size() ),
      .m_ModelMatrix    = Math::RotateY( m_RotationAngle ),
    };

    if ( !m_Renderer.DrawFrame( params ) )
    {
      OnResize();
    }
  }

  void Game::OnResize()
  {
    m_SwapChain.Recreate( m_Window.GetWidth(), m_Window.GetHeight() );
    m_Renderer.OnSwapChainRecreated();
  }

#ifdef _DEBUG
  void Game::UpdateDebugTitle( f32 deltaTime )
  {
    const auto & pos = m_Camera.GetPosition();
    const f32    fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;

    m_Window.SetTitle(
      std::format( L"{} | pos ({:.2f}, {:.2f}, {:.2f}) | {:.2f} fps",
                   m_BaseTitle, pos.m_X, pos.m_Y, pos.m_Z, fps ) );
  }
#endif
} // namespace Anvil::Core
