#pragma once

/*
states.hpp
----------
State functions
*/

#include "Game.hpp"
#include <nwge/state.hpp>

namespace sigmoid {

// Launch state, responsible for picking a game and loading it.
nwge::State *launch(const nwge::StringView &gameName);

// Game main menu state.
nwge::State *gameMenu(Game &&game);

}