#pragma once

/*
StoryScene.hpp
--------------
A scene with a story.
*/

#include <nwge/common/array.hpp>
#include <nwge/common/maybe.hpp>
#include <nwge/common/string.hpp>
#include <nwge/json.hpp>
#include <nwge/render/Texture.hpp>
#include <nwge/render/Vertex.hpp>

namespace sigmoid {

/**
 * @brief Actor definition.
 *
 * This is a more compact, more optimal version of the data structure found in
 * the scene files. `mSprites` is generated from the `sprites` value. Each
 * `sprite` and `speak` command has it's `portrait` value converted from an
 * [x, y] array to an index into the `mSprites` array. This makes it quick for
 * us to render the sprites while also allowing us to easily change the
 * sprites.
 */
struct Actor {
  nwge::String<> id;
  nwge::String<> name;
  nwge::String<> sheet;
  glm::ivec2 sheetSize;

  bool load(const nwge::StringView &actorID, const nwge::json::Object &data);
};

struct Sprite {
  nwge::String<> id;
  bool shown = false;
  glm::vec2 pos{0.5, 0.5};
  glm::vec2 size{-1, -1};
  ssize actor = -1;
  ssize portrait = -1;
};

enum CommandCode {
  CommandInvalid = -1,
  CommandSprite,
  CommandSpeak,
  CommandWait,
  CommandBackground,
  CommandMax,
};

/**
 * @brief Sprite command.
 *
 * While the sprite command uses a string as an ID in the scene file, we use
 * numbers as indices for speed & compactness. We can precalculate how many
 * unique sprites there are and use that to generate the indices. The `actor`
 * field is an index into the `StoryScene::mActors` array.
 */
struct SpriteCommand {
  usize sprite;
  bool hide = false;
  ssize actor = -1;
  ssize portrait = -1;
  glm::vec2 pos{-1, -1};
  glm::vec2 size{-1, -1};

  bool load(const struct StoryScene &scene, const nwge::json::Object &data);
};

struct SpeakCommand {
  nwge::String<> text;
  ssize actor = -1;
  ssize portrait = -1;

  bool load(const struct StoryScene &scene, const nwge::json::Object &data);
};

struct WaitCommand {
  float duration = 0.0f;

  bool load(const nwge::json::Object &data);
};

struct BackgroundCommand {
  bool stopMusic = false;
  bool removeBackground = false;
  ssize image = -1;
  ssize music = -1;

  bool load(const struct StoryScene &scene, const nwge::json::Object &data);
};

struct Command {
  CommandCode code = CommandInvalid;
  nwge::Maybe<SpriteCommand> sprite;
  nwge::Maybe<SpeakCommand> speak;
  nwge::Maybe<WaitCommand> wait;
  nwge::Maybe<BackgroundCommand> background;
};

struct StoryScene {
  nwge::Array<Actor> actors;
  nwge::Array<Sprite> sprites;
  nwge::Array<Command> commands;
  nwge::Array<nwge::String<>> bgImages;
  nwge::Array<nwge::String<>> musicTracks;

  bool load(const nwge::json::Object &data);

private:
  bool loadActors(const nwge::json::Object &data);
  bool loadSprites(const nwge::ArrayView<nwge::json::Value> &commandData);
  bool loadBackgrounds(const nwge::ArrayView<nwge::json::Value> &data);
  bool loadMusicTracks(const nwge::ArrayView<nwge::json::Value> &data);
  bool loadCommands(const nwge::ArrayView<nwge::json::Value> &data);
};

} // namespace sigmoid