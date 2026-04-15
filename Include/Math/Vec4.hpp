#pragma once

#include "Common/Types.hpp"
#include "Math/Vec3.hpp"

namespace Anvil::Math
{
  struct Vec4
  {
    f32 m_X = 0.0f;
    f32 m_Y = 0.0f;
    f32 m_Z = 0.0f;
    f32 m_W = 0.0f;

    constexpr Vec4() = default;

    constexpr Vec4( f32 x, f32 y, f32 z, f32 w )
      : m_X( x )
      , m_Y( y )
      , m_Z( z )
      , m_W( w )
    {
    }

    constexpr explicit Vec4( f32 s )
      : m_X( s )
      , m_Y( s )
      , m_Z( s )
      , m_W( s )
    {
    }

    // Position (w = 1) or direction (w = 0) from a Vec3.
    constexpr Vec4( const Vec3 & v, f32 w )
      : m_X( v.m_X )
      , m_Y( v.m_Y )
      , m_Z( v.m_Z )
      , m_W( w )
    {
    }

    [[nodiscard]] constexpr Vec4 XYZ() const
    {
      return { m_X, m_Y, m_Z, m_W };
    }

    constexpr Vec4 operator+( const Vec4 & rhs ) const
    {
      return { m_X + rhs.m_X, m_Y + rhs.m_Y, m_Z + rhs.m_Z, m_W + rhs.m_W };
    }

    constexpr Vec4 operator-( const Vec4 & rhs ) const
    {
      return { m_X - rhs.m_X, m_Y - rhs.m_Y, m_Z - rhs.m_Z, m_W - rhs.m_W };
    }

    constexpr Vec4 operator*( f32 s ) const
    {
      return { m_X * s, m_Y * s, m_Z * s, m_W * s };
    }

    constexpr Vec4 operator/( f32 s ) const
    {
      return { m_X / s, m_Y / s, m_Z / s, m_W / s };
    }

    constexpr Vec4 operator-() const
    {
      return { -m_X, -m_Y, -m_Z, -m_W };
    }

    [[nodiscard]] constexpr f32 LengthSquared() const
    {
      return m_X * m_X + m_Y * m_Y + m_Z * m_Z;
    }

    [[nodiscard]] f32 Length() const
    {
      return std::sqrt( LengthSquared() );
    }

    static constexpr f32 Dot( const Vec4 & a, const Vec4 & b )
    {
      return a.m_X * b.m_X + a.m_Y * b.m_Y + a.m_Z * b.m_Z + a.m_W * b.m_W;
    }
  };

  constexpr Vec4 operator*( f32 s, const Vec4 & v )
  {
    return v * s;
  }
} // namespace Anvil::Math
