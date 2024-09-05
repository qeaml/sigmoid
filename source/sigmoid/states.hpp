#pragma once

/*
states.hpp
----------
State functions
*/

#include "Game.hpp"
#include "Scene.hpp"
#include <nwge/state.hpp>

namespace sigmoid {

// Launch state, responsible for picking a game and loading it.
nwge::State *launch(const nwge::StringView &gameName);

// Game main menu state.
nwge::State *gameMenu(Game &&game);

// State that handles a scene.
//  This state has two sub states:
//  * One for field scenes
//  * One for story scenes
nwge::State *scene(Game &&game, const nwge::StringView &sceneName);

struct SceneStateData {
  Game &game;
  Scene &scene;
  nwge::render::Font &font;
};

#if 0 // TODO
// Sub state for field scenes.
nwge::SubState *fieldScene(SceneStateData &data);
#endif

// Sub state for story scenes.
nwge::SubState *storyScene(SceneStateData &data);

// Editor game selection state
nwge::State *editorGameSelect();

// Editor game menu state
nwge::State *editorGameMenu(const nwge::StringView &gameName);

} // namespace sigmoid
