#pragma once

#include <vector>

#include "StaticVertex.hpp"

namespace Anvil::Gpu::Primitives
{
  struct MeshData
  {
    std::vector<StaticVertex> m_Vertices;
    std::vector<u16>          m_Indices;
  };

  inline MeshData GenerateCube()
  {
    using V = StaticVertex;

    constexpr i16 N = V::PackSnorm( -1.0f );
    constexpr i16 P = V::PackSnorm( 1.0f );
    constexpr i16 Z = 0;

    constexpr u16 U0 = V::PackUnorm( 0.0f );
    constexpr u16 U1 = V::PackUnorm( 1.0f );

    MeshData mesh;

    mesh.m_Vertices = {
      // Front face (+Z).
      { { N, N, P, Z }, { Z, Z, P, Z }, { U0, U1 } },
      { { P, N, P, Z }, { Z, Z, P, Z }, { U1, U1 } },
      { { P, P, P, Z }, { Z, Z, P, Z }, { U1, U0 } },
      { { N, P, P, Z }, { Z, Z, P, Z }, { U0, U0 } },

      // Back face (-Z).
      { { P, N, N, Z }, { Z, Z, N, Z }, { U0, U1 } },
      { { N, N, N, Z }, { Z, Z, N, Z }, { U1, U1 } },
      { { N, P, N, Z }, { Z, Z, N, Z }, { U1, U0 } },
      { { P, P, N, Z }, { Z, Z, N, Z }, { U0, U0 } },

      // Top face (+Y).
      { { N, P, P, Z }, { Z, P, Z, Z }, { U0, U1 } },
      { { P, P, P, Z }, { Z, P, Z, Z }, { U1, U1 } },
      { { P, P, N, Z }, { Z, P, Z, Z }, { U1, U0 } },
      { { N, P, N, Z }, { Z, P, Z, Z }, { U0, U0 } },

      // Bottom face (-Y).
      { { N, N, N, Z }, { Z, N, Z, Z }, { U0, U1 } },
      { { P, N, N, Z }, { Z, N, Z, Z }, { U1, U1 } },
      { { P, N, P, Z }, { Z, N, Z, Z }, { U1, U0 } },
      { { N, N, P, Z }, { Z, N, Z, Z }, { U0, U0 } },

      // Right face (+X).
      { { P, N, P, Z }, { P, Z, Z, Z }, { U0, U1 } },
      { { P, N, N, Z }, { P, Z, Z, Z }, { U1, U1 } },
      { { P, P, N, Z }, { P, Z, Z, Z }, { U1, U0 } },
      { { P, P, P, Z }, { P, Z, Z, Z }, { U0, U0 } },

      // Left face (-X).
      { { N, N, N, Z }, { N, Z, Z, Z }, { U0, U1 } },
      { { N, N, P, Z }, { N, Z, Z, Z }, { U1, U1 } },
      { { N, P, P, Z }, { N, Z, Z, Z }, { U1, U0 } },
      { { N, P, N, Z }, { N, Z, Z, Z }, { U0, U0 } },
    };

    mesh.m_Indices = {
      0,  1,  2,  2,  3,  0,  // Front
      4,  5,  6,  6,  7,  4,  // Back
      8,  9,  10, 10, 11, 8,  // Top
      12, 13, 14, 14, 15, 12, // Bottom
      16, 17, 18, 18, 19, 16, // Right
      20, 21, 22, 22, 23, 20, // Left
    };

    return mesh;
  }
} // namespace Anvil::Gpu::Primitives
