#pragma once

#include <array>
#include <limits>

#include <vulkan/vulkan.h>

#include "Common/Types.hpp"

namespace Anvil::Gpu
{
  struct StaticVertex
  {
    i16 m_Position[ 4 ] = {}; // SNORM, w unused.
    i16 m_Normal[ 4 ]   = {}; // SNORM, w unused.
    u16 m_Uv[ 2 ]       = {}; // UNORM

    static constexpr VkVertexInputBindingDescription GetBindingDescription()
    {
      return { .binding   = 0,
               .stride    = sizeof( StaticVertex ),
               .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
    }

    static constexpr std::array<VkVertexInputAttributeDescription, 3>
    GetAttributeDescriptions()
    {
      return { { { .location = 0,
                   .binding  = 0,
                   .format   = VK_FORMAT_R16G16B16A16_SNORM,
                   .offset   = offsetof( StaticVertex, m_Position ) },
                 { .location = 1,
                   .binding  = 0,
                   .format   = VK_FORMAT_R16G16B16A16_SNORM,
                   .offset   = offsetof( StaticVertex, m_Normal ) },
                 { .location = 2,
                   .binding  = 0,
                   .format   = VK_FORMAT_R16G16_UNORM,
                   .offset   = offsetof( StaticVertex, m_Uv ) } } };
    }

    // Packs a float in [-1, 1] to SNORM i16.
    static constexpr i16 PackSnorm( f32 value )
    {
      return static_cast<i16>(
        value < 0.0f
          ? value * static_cast<f32>( -std::numeric_limits<i16>::min() )
          : value * static_cast<f32>( std::numeric_limits<i16>::max() ) );
    }

    // Packs a float in [0, 1] to UNORM u16.
    static constexpr u16 PackUnorm( f32 value )
    {
      return static_cast<u16>(
        value * static_cast<f32>( std::numeric_limits<u16>::max() ) );
    }
  };

  static_assert( sizeof( StaticVertex ) == 20 );
} // namespace Anvil::Gpu
