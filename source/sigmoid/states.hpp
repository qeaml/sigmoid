#pragma once

/*
states.hpp
----------
State functions
*/

#include "AssetManager.hpp"
#include "Game.hpp"
#include "Scene.hpp"
#include "SceneManager.hpp"
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

struct EditorInfo {
  nwge::String<> gameName;
  nwge::data::Store store;
  AssetManager assets;
  SceneManager scenes;
  Game game;
};

nwge::State *editorState(const nwge::StringView &gameName);
nwge::SubState *gameInfoEditor(EditorInfo &info);
nwge::SubState *sceneEditor(EditorInfo &info, const nwge::StringView &sceneName, SceneType defaultType = SceneInvalid);

inline void setEditorSubState(nwge::SubState *state) {
  nwge::swapSubStatePtr(state, {
    .tickParent = true,
  });
}

} // namespace sigmoid
