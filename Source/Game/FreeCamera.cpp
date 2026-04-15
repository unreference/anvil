#include <numbers>
#include <algorithm>

#include "Game/FreeCamera.hpp"
#include "Math/Transforms.hpp"

namespace Anvil::Game
{
  static constexpr f32 MaxPitch = std::numbers::pi_v<f32> / 2.0f - 0.01f;

  FreeCamera::FreeCamera( const Math::Vec3 & position )
    : m_Position( position )
  {
  }

  void FreeCamera::Update( f32 deltaTime, const CameraInput & input )
  {
    m_Yaw   += input.m_MouseDeltaX * m_Sensitivity;
    m_Pitch += input.m_MouseDeltaY * m_Sensitivity;
    m_Pitch  = std::clamp( m_Pitch, -MaxPitch, MaxPitch );

    const f32 speed =
      m_MoveSpeed * ( input.m_IsSprinting ? m_SprintFactor : 1.0f ) * deltaTime;

    // Movement is relative to the camera's horizontal orientation.
    const Math::Vec3 forward = GetForward();
    const Math::Vec3 right   = GetRight();

    // Project forward onto the horizontal plane for WASD movement.
    const Math::Vec3 flatForward =
      Math::Vec3( forward.m_X, 0.0f, forward.m_Z ).Normalized();

    if ( input.m_IsMovingForward )
    {
      m_Position += flatForward * speed;
    }

    if ( input.m_IsMovingBack )
    {
      m_Position -= flatForward * speed;
    }

    if ( input.m_IsMovingRight )
    {
      m_Position += right * speed;
    }

    if ( input.m_IsMovingLeft )
    {
      m_Position -= right * speed;
    }

    if ( input.m_IsMovingUp )
    {
      m_Position.m_Y += speed;
    }

    if ( input.m_IsMovingDown )
    {
      m_Position.m_Y -= speed;
    }
  }

  Math::Mat4 FreeCamera::GetViewMatrix() const
  {
    return Math::LookAt( m_Position, m_Position + GetForward(),
                         { 0.0f, 1.0f, 0.0f } );
  }

  Math::Mat4 FreeCamera::GetProjectionMatrix( f32 aspect ) const
  {
    return Math::Perspective( Math::Radians( m_Fov ), aspect, m_NearPlane,
                              m_FarPlane );
  }

  Math::Vec3 FreeCamera::GetForward() const
  {
    const f32 cosPitch = std::cos( m_Pitch );
    return { cosPitch * std::sin( m_Yaw ), std::sin( m_Pitch ),
             -cosPitch * std::cos( m_Yaw ) };
  }

  Math::Vec3 FreeCamera::GetRight() const
  {
    return { std::cos( m_Yaw ), 0.0f, std::sin( m_Yaw ) };
  }
} // namespace Anvil::Game
