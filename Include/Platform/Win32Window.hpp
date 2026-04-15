#pragma once

#include <Windows.h>

#include <string_view>

#include "Common/Types.hpp"

namespace Anvil::Platform
{
  struct WindowConfig
  {
    std::wstring_view m_Title  = L"Anvil";
    u32               m_Width  = 1280;
    u32               m_Height = 720;
  };

  class Win32Window
  {
  public:
    ANVIL_NO_COPY_NO_MOVE( Win32Window );

    explicit Win32Window( const WindowConfig & config = {} );
    ~Win32Window();

    // Pumps the Win32 message queue.
    // Returns false when the window should close.
    bool PollEvents();

    [[nodiscard]] HWND GetHandle() const
    {
      return m_Window;
    }

    [[nodiscard]] HINSTANCE GetInstance() const
    {
      return m_Instance;
    }

    [[nodiscard]] u32 GetWidth() const
    {
      return m_Width;
    }

    [[nodiscard]] u32 GetHeight() const
    {
      return m_Height;
    }

    [[nodiscard]] bool IsResized() const
    {
      return m_IsResized;
    }

    void ClearResizedFlag()
    {
      m_IsResized = false;
    }

  private:
    static LRESULT CALLBACK WindowProc( HWND hwnd, UINT msg, WPARAM wp,
                                        LPARAM lp );

    HWND      m_Window = nullptr;
    HINSTANCE m_Instance;
    u32       m_Width     = 0;
    u32       m_Height    = 0;
    bool      m_IsResized = false;
    bool      m_IsClosed  = false;
  };
} // namespace Anvil::Platform
