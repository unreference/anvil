#pragma once

#include <vector>

#include "Common/Types.hpp"
#include "Texture.hpp"

namespace Anvil::Gpu
{
  struct CheckerboardResult
  {
    std::vector<u8> m_Pixels;
    TextureDesc     m_Desc;
  };

  inline CheckerboardResult GenerateCheckerboard( u32 size     = 256,
                                                  u32 cellSize = 16,
                                                  u32 colorA   = 0xFFFFFFFF,
                                                  u32 colorB   = 0xFF888888 )
  {
    CheckerboardResult result;
    result.m_Desc = { .m_Width = size, .m_Height = size };

    result.m_Pixels.resize( static_cast<usize>( size ) * size * 4 );

    for ( u32 y = 0; y < size; ++y )
    {
      for ( u32 x = 0; x < size; ++x )
      {
        const bool isEven = ( x / cellSize + y / cellSize ) % 2 == 0;
        const u32  color  = isEven ? colorA : colorB;

        const usize index               = static_cast<usize>(y * size + x) * 4;
        result.m_Pixels.at( index + 0 ) = static_cast<u8>( color >> 0 );
        result.m_Pixels.at( index + 1 ) = static_cast<u8>( color >> 8 );
        result.m_Pixels.at( index + 2 ) = static_cast<u8>( color >> 16 );
        result.m_Pixels.at( index + 3 ) = static_cast<u8>( color >> 24 );
      }
    }

    return result;
  }
} // namespace Anvil::Gpu
