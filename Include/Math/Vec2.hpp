#pragma once

#include <cmath>

#include "Common/Types.hpp"

namespace Anvil::Math
{
  struct Vec2
  {
    f32 m_X = 0.0f;
    f32 m_Y = 0.0f;

    constexpr Vec2() = default;

    constexpr Vec2( f32 x, f32 y )
      : m_X( x )
      , m_Y( y )
    {
    }

    constexpr explicit Vec2( f32 s )
      : m_X( s )
      , m_Y( s )
    {
    }

    constexpr Vec2 operator+( const Vec2 & rhs ) const
    {
      return { m_X + rhs.m_X, m_Y + rhs.m_Y };
    }

    constexpr Vec2 operator-( const Vec2 & rhs ) const
    {
      return { m_X - rhs.m_X, m_Y - rhs.m_Y };
    }

    constexpr Vec2 operator*( f32 s ) const
    {
      return { m_X * s, m_Y * s };
    }

    constexpr Vec2 operator/( f32 s ) const
    {
      return { m_X / s, m_Y / s };
    }

    constexpr Vec2 operator-() const
    {
      return { -m_X, -m_Y };
    }

    constexpr Vec2 operator+=( const Vec2 & rhs )
    {
      m_X += rhs.m_X;
      m_Y += rhs.m_Y;
      return *this;
    }

    constexpr Vec2 operator-=( const Vec2 & rhs )
    {
      m_X -= rhs.m_X;
      m_Y -= rhs.m_Y;
      return *this;
    }

    constexpr Vec2 operator*=( f32 s )
    {
      m_X *= s;
      m_Y *= s;
      return *this;
    }

    [[nodiscard]] constexpr f32 LengthSquared() const
    {
      return m_X * m_X + m_Y * m_Y;
    }

    [[nodiscard]] f32 Length() const
    {
      return std::sqrt( LengthSquared() );
    }

    [[nodiscard]] Vec2 Normalized() const
    {
      const f32 length = Length();
      if ( length < 1e-8f )
      {
        return {};
      }

      return *this / length;
    }

    static constexpr f32 Dot( const Vec2 & a, const Vec2 & b )
    {
      return a.m_X * b.m_X + a.m_Y * b.m_Y;
    }
  };

  constexpr Vec2 operator*( f32 s, const Vec2 & v )
  {
    return v * s;
  }
} // namespace Anvil::Math
