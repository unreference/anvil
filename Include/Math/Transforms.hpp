#pragma once

#include <cmath>
#include <numbers>

#include "Math/Mat4.hpp"
#include "Math/Vec3.hpp"

namespace Anvil::Math
{
  inline Mat4 Perspective( f32 fovRadians, f32 aspect, f32 near, f32 far )
  {
    const f32 tanHalf = std::tan( fovRadians * 0.5f );

    Mat4 result;
    result.m_M.at( 0 ).at( 0 ) = 1.0f / ( aspect * tanHalf );
    result.m_M.at( 1 ).at( 1 ) = -1.0f / tanHalf;
    result.m_M.at( 2 ).at( 2 ) = far / ( near - far );
    result.m_M.at( 2 ).at( 3 ) = -1.0f;
    result.m_M.at( 3 ).at( 2 ) = near * far / ( near - far );
    return result;
  }

  inline Mat4 LookAt( const Vec3 & eye, const Vec3 & target, const Vec3 & up )
  {
    const Vec3 forward = ( target - eye ).Normalized();
    const Vec3 right   = Vec3::Cross( forward, up ).Normalized();
    const Vec3 camUp   = Vec3::Cross( right, forward );

    Mat4 result( 1.0f );
    result.m_M.at( 0 ).at( 0 ) = right.m_X;
    result.m_M.at( 1 ).at( 0 ) = right.m_Y;
    result.m_M.at( 2 ).at( 0 ) = right.m_Z;

    result.m_M.at( 0 ).at( 1 ) = camUp.m_X;
    result.m_M.at( 1 ).at( 1 ) = camUp.m_Y;
    result.m_M.at( 2 ).at( 1 ) = camUp.m_Z;

    result.m_M.at( 0 ).at( 2 ) = -forward.m_X;
    result.m_M.at( 1 ).at( 2 ) = -forward.m_Y;
    result.m_M.at( 2 ).at( 2 ) = -forward.m_Z;

    result.m_M.at( 3 ).at( 0 ) = -Vec3::Dot( right, eye );
    result.m_M.at( 3 ).at( 1 ) = -Vec3::Dot( camUp, eye );
    result.m_M.at( 3 ).at( 2 ) = Vec3::Dot( forward, eye );
    return result;
  }

  constexpr Mat4 Translate( const Vec3 & offset )
  {
    Mat4 result( 1.0f );
    result.m_M.at( 3 ).at( 0 ) = offset.m_X;
    result.m_M.at( 3 ).at( 1 ) = offset.m_Y;
    result.m_M.at( 3 ).at( 2 ) = offset.m_Z;
    return result;
  }

  inline Mat4 RotateX( f32 radians )
  {
    const f32 cos = std::cos( radians );
    const f32 sin = std::sin( radians );

    Mat4 result( 1.0f );
    result.m_M.at( 1 ).at( 1 ) = cos;
    result.m_M.at( 1 ).at( 2 ) = sin;
    result.m_M.at( 2 ).at( 1 ) = -sin;
    result.m_M.at( 2 ).at( 2 ) = cos;
    return result;
  }

  inline Mat4 RotateY( f32 radians )
  {
    const f32 cos = std::cos( radians );
    const f32 sin = std::sin( radians );

    Mat4 result( 1.0f );
    result.m_M.at( 0 ).at( 0 ) = cos;
    result.m_M.at( 0 ).at( 2 ) = -sin;
    result.m_M.at( 2 ).at( 0 ) = sin;
    result.m_M.at( 2 ).at( 2 ) = cos;
    return result;
  }

  inline Mat4 RotateZ( f32 radians )
  {
    const f32 cos = std::cos( radians );
    const f32 sin = std::sin( radians );

    Mat4 result( 1.0f );
    result.m_M.at( 0 ).at( 0 ) = cos;
    result.m_M.at( 0 ).at( 1 ) = sin;
    result.m_M.at( 1 ).at( 0 ) = -sin;
    result.m_M.at( 1 ).at( 1 ) = cos;
    return result;
  }

  constexpr Mat4 Scale( const Vec3 & s )
  {
    Mat4 result;
    result.m_M.at( 0 ).at( 0 ) = s.m_X;
    result.m_M.at( 1 ).at( 1 ) = s.m_Y;
    result.m_M.at( 2 ).at( 2 ) = s.m_Z;
    result.m_M.at( 3 ).at( 3 ) = 1.0f;
    return result;
  }

  constexpr f32 Radians( f32 degrees )
  {
    return degrees * std::numbers::pi_v<f32> / 180.0f;
  }
} // namespace Anvil::Math
