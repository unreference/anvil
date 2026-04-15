#pragma once

#include "Common/Types.hpp"
#include "Math/Vec3.hpp"
#include "CameraInput.hpp"
#include "Math/Mat4.hpp"

namespace Anvil::Gameplay
{
  class FreeCamera
  {
  public:
    explicit FreeCamera( const Math::Vec3 & position = {} );
    ~FreeCamera() = default;

    void Update( f32 deltaTime, const CameraInput & input );

    [[nodiscard]] Math::Mat4 GetViewMatrix() const;
    [[nodiscard]] Math::Mat4 GetProjectionMatrix( f32 aspect ) const;

    void SetPosition( const Math::Vec3 & position )
    {
      m_Position = position;
    }

    [[nodiscard]] const Math::Vec3 & GetPosition() const
    {
      return m_Position;
    }

    f32 m_MoveSpeed    = 5.0f;
    f32 m_SprintFactor = 3.0f;
    f32 m_Sensitivity  = 0.002f;
    f32 m_Fov          = 60.0f;
    f32 m_NearPlane    = 0.1f;
    f32 m_FarPlane     = 1000.0f;

  private:
    [[nodiscard]] Math::Vec3 GetForward() const;
    [[nodiscard]] Math::Vec3 GetRight() const;

    Math::Vec3 m_Position;
    f32        m_Yaw   = 0.0f;
    f32        m_Pitch = 0.0f;
  };
} // namespace Anvil::Game
