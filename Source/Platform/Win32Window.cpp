#include <stdexcept>

#include "Platform/Win32Window.hpp"

namespace Anvil::Platform
{
  static constexpr c16 WindowClassName[] = L"AnvilWindowClass";

  Win32Window::Win32Window( const WindowConfig & config /*= {} */ )
    : m_Instance( GetModuleHandleW( nullptr ) )
    , m_Width( config.m_Width )
    , m_Height( config.m_Height )
  {
    // Per-monitor DPI awareness so the window isn't blurry on high-DPI displays.
    SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

    const WNDCLASSEXW wc = { .cbSize        = sizeof( WNDCLASSEXW ),
                             .style         = CS_HREDRAW | CS_VREDRAW,
                             .lpfnWndProc   = WindowProc,
                             .hInstance     = m_Instance,
                             .hCursor       = LoadCursorW( nullptr, IDC_ARROW ),
                             .lpszClassName = WindowClassName };

    if ( !RegisterClassExW( &wc ) )
    {
      throw std::runtime_error( "Failed to register window class." );
    }

    // Size the window so the client area matches the requested dimensions.
    RECT rect = { 0, 0, static_cast<LONG>( m_Width ),
                  static_cast<LONG>( m_Height ) };
    AdjustWindowRectEx( &rect, WS_OVERLAPPEDWINDOW, FALSE, 0 );

    m_Window = CreateWindowExW(
      0, WindowClassName, config.m_Title.data(), WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
      rect.bottom - rect.top, nullptr, nullptr, m_Instance, this );

    if ( !m_Window )
    {
      UnregisterClassW( WindowClassName, m_Instance );
      throw std::runtime_error( "Failed to create window." );
    }

    RegisterRawInput();
    ShowWindow( m_Window, SW_SHOW );
  }

  Win32Window::~Win32Window()
  {
    if ( m_IsCursorCaptured )
    {
      SetCursorCaptured( false );
    }

    if ( m_Window )
    {
      DestroyWindow( m_Window );
    }

    UnregisterClassW( WindowClassName, m_Instance );
  }

  bool Win32Window::PollEvents()
  {
    MSG msg = {};
    while ( PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE ) )
    {
      if ( msg.message == WM_QUIT )
      {
        m_IsClosed = true;
        return false;
      }

      TranslateMessage( &msg );
      DispatchMessageW( &msg );
    }

    return !m_IsClosed;
  }
  void Win32Window::ResetFrameInput()
  {
    m_MouseDelta = {};
  }
  void Win32Window::SetCursorCaptured( bool isCaptured )
  {
    m_IsCursorCaptured = isCaptured;

    if ( isCaptured )
    {
      ShowCursor( FALSE );
      ClipCursorToWindow();
    }
    else
    {
      ShowCursor( TRUE );
      ClipCursor( nullptr );
    }
  }
  void Win32Window::RegisterRawInput()
  {
    // Raw mouse input avoids cursor clamping and OS acceleration.
    const RAWINPUTDEVICE rid = { .usUsagePage = 0x01,
                                 .usUsage     = 0x02,
                                 .dwFlags     = 0,
                                 .hwndTarget  = m_Window };

    RegisterRawInputDevices( &rid, 1, sizeof( rid ) );
  }
  void Win32Window::ClipCursorToWindow()
  {
    RECT clientRect;
    GetClientRect( m_Window, &clientRect );

    POINT topLeft     = { clientRect.left, clientRect.top };
    POINT bottomRight = { clientRect.right, clientRect.bottom };
    ClientToScreen( m_Window, &topLeft );
    ClientToScreen( m_Window, &bottomRight );

    const RECT screenRect = { topLeft.x, topLeft.y, bottomRight.x,
                              bottomRight.y };
    ClipCursor( &screenRect );
  }

  LRESULT CALLBACK Win32Window::WindowProc( HWND hwnd, UINT msg, WPARAM wp,
                                            LPARAM lp )
  {
    Win32Window * window = nullptr;

    if ( msg == WM_CREATE )
    {
      auto * cs = reinterpret_cast<CREATESTRUCTW *>( lp );
      window    = static_cast<Win32Window *>( cs->lpCreateParams );
      SetWindowLongPtrW( hwnd, GWLP_USERDATA,
                         reinterpret_cast<LONG_PTR>( window ) );
    }
    else
    {
      window = reinterpret_cast<Win32Window *>(
        GetWindowLongPtrW( hwnd, GWLP_USERDATA ) );
    }

    if ( !window )
    {
      return DefWindowProcW( hwnd, msg, wp, lp );
    }

    switch ( msg )
    {
      case WM_SIZE:
      {
        const auto width  = static_cast<u32>( LOWORD( lp ) );
        const auto height = static_cast<u32>( HIWORD( lp ) );

        if ( width != window->m_Width || height != window->m_Height )
        {
          window->m_Width  = width;
          window->m_Height = height;

          if ( width > 0 && height > 0 )
          {
            window->m_IsResized = true;
          }
        }

        if ( window->m_IsCursorCaptured )
        {
          window->ClipCursorToWindow();
        }

        return 0;
      }

      case WM_KEYDOWN:
      {
        if ( wp < window->m_KeyState.size() )
        {
          window->m_KeyState.at( wp ) = true;
        }

        return 0;
      }

      case WM_KEYUP:
      {
        if ( wp < window->m_KeyState.size() )
        {
          window->m_KeyState.at( wp ) = false;
        }

        return 0;
      }

      case WM_LBUTTONDOWN:
      {
        window->m_KeyState.at( VK_LBUTTON ) = true;
        return 0;
      }

      case WM_LBUTTONUP:
      {
        window->m_KeyState.at( VK_LBUTTON ) = false;
        return 0;
      }

      case WM_RBUTTONDOWN:
      {
        window->m_KeyState.at( VK_RBUTTON ) = true;
        return 0;
      }

      case WM_RBUTTONUP:
      {
        window->m_KeyState.at( VK_RBUTTON ) = false;
        return 0;
      }

      case WM_MBUTTONDOWN:
      {
        window->m_KeyState.at( VK_MBUTTON ) = true;
        return 0;
      }

      case WM_MBUTTONUP:
      {
        window->m_KeyState.at( VK_MBUTTON ) = false;
        return 0;
      }

      case WM_INPUT:
      {
        if ( !window->m_IsCursorCaptured )
        {
          break;
        }

        UINT     size = sizeof( RAWINPUT );
        RAWINPUT raw  = {};
        GetRawInputData( reinterpret_cast<HRAWINPUT>( lp ), RID_INPUT, &raw,
                         &size, sizeof( RAWINPUTHEADER ) );

        if ( raw.header.dwType == RIM_TYPEMOUSE )
        {
          window->m_MouseDelta.m_X += static_cast<f32>( raw.data.mouse.lLastX );
          window->m_MouseDelta.m_Y += static_cast<f32>( raw.data.mouse.lLastY );
        }

        return 0;
      }

      case WM_CLOSE:
      {
        window->m_IsClosed = true;
        return 0;
      }

      case WM_DESTROY:
      {
        PostQuitMessage( 0 );
        return 0;
      }

      default:
      {
        break;
      }
    }

    return DefWindowProcW( hwnd, msg, wp, lp );
  }
} // namespace Anvil::Platform
