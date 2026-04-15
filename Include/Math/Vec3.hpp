#pragma once

#include "Common/Types.hpp"

namespace Anvil::Math
{
  struct Vec3
  {
    f32 m_X = 0.0f;
    f32 m_Y = 0.0f;
    f32 m_Z = 0.0f;

    constexpr Vec3() = default;

    constexpr Vec3( f32 x, f32 y, f32 z )
      : m_X( x )
      , m_Y( y )
      , m_Z( z )
    {
    }

    constexpr explicit Vec3( f32 s )
      : m_X( s )
      , m_Y( s )
      , m_Z( s )
    {
    }

    constexpr Vec3 operator+( const Vec3 & rhs ) const
    {
      return { m_X + rhs.m_X, m_Y + rhs.m_Y, m_Z + rhs.m_Z };
    }

    constexpr Vec3 operator-( const Vec3 & rhs ) const
    {
      return { m_X - rhs.m_X, m_Y - rhs.m_Y, m_Z - rhs.m_Z };
    }

    constexpr Vec3 operator*( f32 s ) const
    {
      return { m_X * s, m_Y * s, m_Z * s };
    }

    constexpr Vec3 operator/( f32 s ) const
    {
      return { m_X / s, m_Y / s, m_Z / s };
    }

    constexpr Vec3 operator-() const
    {
      return { -m_X, -m_Y, -m_Z };
    }

    constexpr Vec3 & operator+=( const Vec3 & rhs )
    {
      m_X += rhs.m_X;
      m_Y += rhs.m_Y;
      m_Z += rhs.m_Z;
      return *this;
    }

    constexpr Vec3 & operator-=( const Vec3 & rhs )
    {
      m_X -= rhs.m_X;
      m_Y -= rhs.m_Y;
      m_Z -= rhs.m_Z;
      return *this;
    }

    constexpr Vec3 & operator*=( f32 s )
    {
      m_X *= s;
      m_Y *= s;
      m_Z *= s;
      return *this;
    }

    [[nodiscard]] constexpr f32 LengthSquared() const
    {
      return m_X * m_X + m_Y * m_Y + m_Z * m_Z;
    }

    [[nodiscard]] f32 Length() const
    {
      return std::sqrt( LengthSquared() );
    }

    [[nodiscard]] Vec3 Normalized() const
    {
      const f32 length = Length();
      if ( length < 1e-8f )
      {
        return {};
      }

      return *this / length;
    }

    static constexpr f32 Dot( const Vec3 & a, const Vec3 & b )
    {
      return a.m_X * b.m_X + a.m_Y * b.m_Y + a.m_Z * b.m_Z;
    }

    static constexpr Vec3 Cross( const Vec3 & a, const Vec3 & b )
    {
      return {
        a.m_Y * b.m_Z - a.m_Z * b.m_Y,
        a.m_Z * b.m_X - a.m_X * b.m_Z,
        a.m_X * b.m_Y - a.m_Y * b.m_X,
      };
    }
  };

  constexpr Vec3 operator*( f32 s, const Vec3 & v )
  {
    return v * s;
  }
} // namespace Anvil::Math
