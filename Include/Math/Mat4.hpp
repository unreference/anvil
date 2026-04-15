#pragma once

#include <array>

#include "Common/Types.hpp"
#include "Math/Vec4.hpp"

namespace Anvil::Math
{
  struct Mat4
  {
    // Column-major: m_M[ column ][ row ].  Matches GLSL/Vulkan layout.
    std::array<std::array<f32, 4>, 4> m_M = {};

    constexpr Mat4() = default;

    // Diagonal constructor - Mat4( 1.0f ) gives identity.
    constexpr explicit Mat4( f32 diagonal )
    {
      m_M.at( 0 ).at( 0 ) = diagonal;
      m_M.at( 1 ).at( 1 ) = diagonal;
      m_M.at( 2 ).at( 2 ) = diagonal;
      m_M.at( 3 ).at( 3 ) = diagonal;
    }

    // Construct from four column vectors.
    constexpr Mat4( const Vec4 & c0, const Vec4 & c1, const Vec4 & c2,
                    const Vec4 & c3 )
    {
      m_M.at( 0 ) = { c0.m_X, c0.m_Y, c0.m_Z, c0.m_W };
      m_M.at( 1 ) = { c1.m_X, c1.m_Y, c1.m_Z, c1.m_W };
      m_M.at( 2 ) = { c2.m_X, c2.m_Y, c2.m_Z, c2.m_W };
      m_M.at( 3 ) = { c3.m_X, c3.m_Y, c3.m_Z, c3.m_W };
    }

    static constexpr Mat4 Identity()
    {
      return Mat4( 1.0f );
    }

    [[nodiscard]] constexpr Vec4 Column( usize index ) const
    {
      const auto & column = m_M.at( index );
      return { column.at( 0 ), column.at( 1 ), column.at( 2 ), column.at( 3 ) };
    }

    [[nodiscard]] constexpr Vec4 Row( usize index ) const
    {
      return {
        m_M.at( 0 ).at( index ),
        m_M.at( 1 ).at( index ),
        m_M.at( 2 ).at( index ),
        m_M.at( 3 ).at( index ),
      };
    }

    // Raw pointer for GPU upload (push constants, UBOs)
    [[nodiscard]] constexpr const f32 * Data() const
    {
      return m_M.at( 0 ).data();
    }

    constexpr Mat4 operator*( const Mat4 & rhs ) const
    {
      Mat4 result;

      for ( usize column = 0; column < 4; ++column )
      {
        for ( usize row = 0; row < 4; ++row )
        {
          result.m_M.at( column ).at( row ) =
            m_M.at( 0 ).at( row ) * rhs.m_M.at( column ).at( 0 ) +
            m_M.at( 1 ).at( row ) * rhs.m_M.at( column ).at( 1 ) +
            m_M.at( 2 ).at( row ) * rhs.m_M.at( column ).at( 2 ) +
            m_M.at( 3 ).at( row ) * rhs.m_M.at( column ).at( 3 );
        }
      }

      return result;
    }

    constexpr Vec4 operator*( const Vec4 & v ) const
    {
      return {
        m_M.at( 0 ).at( 0 ) * v.m_X + m_M.at( 1 ).at( 0 ) * v.m_Y +
          m_M.at( 2 ).at( 0 ) * v.m_Z + m_M.at( 3 ).at( 0 ) * v.m_W,

        m_M.at( 0 ).at( 1 ) * v.m_X + m_M.at( 1 ).at( 1 ) * v.m_Y +
          m_M.at( 2 ).at( 1 ) * v.m_Z + m_M.at( 3 ).at( 1 ) * v.m_W,

        m_M.at( 0 ).at( 2 ) * v.m_X + m_M.at( 1 ).at( 2 ) * v.m_Y +
          m_M.at( 2 ).at( 2 ) * v.m_Z + m_M.at( 3 ).at( 2 ) * v.m_W,

        m_M.at( 0 ).at( 3 ) * v.m_X + m_M.at( 1 ).at( 3 ) * v.m_Y +
          m_M.at( 2 ).at( 3 ) * v.m_Z + m_M.at( 3 ).at( 3 ) * v.m_W,
      };
    }

    [[nodiscard]] constexpr Mat4 Transposed() const
    {
      Mat4 result;

      for ( usize column = 0; column < 4; ++column )
      {
        for ( usize row = 0; row < 4; ++row )
        {
          result.m_M.at( column ).at( row ) = m_M.at( row ).at( column );
        }
      }

      return result;
    }
  };
} // namespace Anvil::Math
