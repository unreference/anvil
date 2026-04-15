#include <exception>
#include <iostream>
#include <chrono>

#include "Core/Game.hpp"

int main()
{
  try
  {
    Anvil::Core::Game game;
    game.Run();
  }
  catch ( const std::exception & e )
  {
    std::cerr << std::format( "[Fatal] {}\n", e.what() );
    return 1;
  }

  return 0;
}
