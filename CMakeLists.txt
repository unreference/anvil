cmake_minimum_required( VERSION 4.2.3 )
project( Anvil LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# --------------------------------------------------------------------------- #
# Sources
# --------------------------------------------------------------------------- #

set( ANVIL_SOURCES
  Source/Main.cpp
  Source/Gpu/Renderer.cpp
  Source/Gpu/SwapChain.cpp
  Source/Gpu/VulkanContext.cpp
  Source/Platform/Win32Window.cpp
)

set( ANVIL_HEADERS
  Include/Common/Types.hpp
  Include/Gpu/Renderer.hpp
  Include/Gpu/SwapChain.hpp
  Include/Gpu/VulkanContext.hpp
  Include/Platform/Win32Window.hpp
)

add_executable( Anvil ${ANVIL_SOURCES} ${ANVIL_HEADERS} )

target_include_directories( Anvil PRIVATE ${CMAKE_SOURCE_DIR}/Include )

# --------------------------------------------------------------------------- #
# Vulkan
# --------------------------------------------------------------------------- #

find_package( Vulkan REQUIRED )
target_link_libraries( Anvil PRIVATE Vulkan::Vulkan )

# --------------------------------------------------------------------------- #
# Platform defines
# --------------------------------------------------------------------------- #

target_compile_definitions( Anvil PRIVATE
  WIN32_LEAN_AND_MEAN
  NOMINMAX
  VK_USE_PLATFORM_WIN32_KHR
  UNICODE
  _UNICODE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
)

# --------------------------------------------------------------------------- #
# Compiler flags (MSVC)
# --------------------------------------------------------------------------- #

if ( MSVC )
  # Shared across both configs.
  target_compile_options( Anvil PRIVATE
    /W4           # High warning level.
    /WX           # Warnings as errors.
    /permissive-  # Strict conformance mode.
    /sdl          # Security Development Lifecycle checks.
    /GR-          # No RTTI.
    /EHsc         # Standard C++ exception handling.
    /utf-8        # Source and execution charset.
  )

  # Debug
  target_compile_options( Anvil PRIVATE
    $<$<CONFIG:Debug>:
      /Zi         # Full debug info.
      /Od         # No optimization.
      /RTC1       # Runtime error checks (stack frame + uninitialized).
    >
  )

  # Release
  target_compile_options( Anvil PRIVATE
    $<$<CONFIG:Release>:
      /Zi         # Debug info for profiling and crash dumps.
      /O2         # Maximize speed.
      /Oi         # Enable intrinsic functions.
      /GL         # Whole program optimization.
      /Gy         # Function-level linking.
    >
  )

  # Release linker flags.
  target_link_options( Anvil PRIVATE
    $<$<CONFIG:Release>:
      /LTCG       # Link-time code generation.
      /OPT:REF    # Eliminate unreferenced code.
      /OPT:ICF    # Identical COMDAT folding.
    >
  )

  # Console subsystem.
  target_link_options( Anvil PRIVATE /SUBSYSTEM:CONSOLE )
endif()
