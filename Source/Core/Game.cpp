#include <format>

#include "Core/Game.hpp"
#include "Common/Version.hpp"

#include <chrono>
#include <thread>

namespace Anvil::Core
{
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

  Game::Game()
    : m_BaseTitle( BuildTitle() )
    , m_Window( { .m_Title = m_BaseTitle } )
    , m_Gpu( m_Window.GetHandle(), m_Window.GetInstance() )
    , m_SwapChain( m_Gpu, m_Window.GetWidth(), m_Window.GetHeight() )
    , m_Renderer( m_Gpu, m_SwapChain )
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

    if ( !m_Renderer.DrawFrame() )
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
