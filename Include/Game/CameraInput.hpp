#pragma once

#include "Common/Types.hpp"

namespace Anvil::Game
{
  struct CameraInput
  {
    bool m_IsMovingForward = false;
    bool m_IsMovingBack    = false;
    bool m_IsMovingLeft    = false;
    bool m_IsMovingRight   = false;
    bool m_IsMovingUp      = false;
    bool m_IsMovingDown    = false;
    bool m_IsSprinting     = false;

    f32 m_MouseDeltaX = 0.0f;
    f32 m_MouseDeltaY = 0.0f;
  };
} // namespace Anvil::Game
