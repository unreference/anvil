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

    ShowWindow( m_Window, SW_SHOW );
  }

  Win32Window::~Win32Window()
  {
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

        if ( width > 0 && height > 0 &&
             ( width != window->m_Width || height != window->m_Height ) )
        {
          window->m_Width     = width;
          window->m_Height    = height;
          window->m_IsResized = true;
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
