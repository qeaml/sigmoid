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
#include <nwge/json/Schema.hpp>
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
  [[nodiscard]]
  nwge::json::Object toObject() const;
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

static constexpr std::array cCommandCodeNames = {
  "Sprite",
  "Speak",
  "Wait",
  "Background",
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

  bool load(const struct StoryScene &scene, nwge::json::Schema &data);
  [[nodiscard]]
  nwge::json::Object toObject(const StoryScene &scene) const;
};

struct SpeakCommand {
  nwge::String<> text;
  ssize actor = -1;
  ssize portrait = -1;

  bool load(const struct StoryScene &scene, nwge::json::Schema &data);
  [[nodiscard]]
  nwge::json::Object toObject(const StoryScene &scene) const;
};

struct WaitCommand {
  float duration = 0.0f;

  bool load(nwge::json::Schema &data);
  [[nodiscard]]
  nwge::json::Object toObject() const;
};

struct BackgroundCommand {
  bool stopMusic = false;
  bool removeBackground = false;
  ssize image = -1;
  ssize music = -1;

  bool load(const struct StoryScene &scene, nwge::json::Schema &data);
  [[nodiscard]]
  nwge::json::Object toObject(const StoryScene &scene) const;
};

struct Command {
  CommandCode code = CommandInvalid;
  nwge::Maybe<SpriteCommand> sprite;
  nwge::Maybe<SpeakCommand> speak;
  nwge::Maybe<WaitCommand> wait;
  nwge::Maybe<BackgroundCommand> background;

  [[nodiscard]]
  nwge::json::Object toObject(const StoryScene &scene) const;
};

struct StoryScene {
  nwge::Array<Actor> actors;
  nwge::Array<Sprite> sprites;
  nwge::Array<Command> commands;
  nwge::Array<nwge::String<>> bgImages;
  nwge::Array<nwge::String<>> musicTracks;

  bool load(nwge::json::Schema &root);
  [[nodiscard]]
  nwge::json::Object toObject() const;

private:
  bool loadActors(nwge::json::Schema &data);
  [[nodiscard]]
  nwge::json::Object actorsObject() const;
  bool loadSprites(nwge::json::Schema data);
  bool loadBackgrounds(nwge::json::Schema data);
  bool loadCommands(nwge::json::Schema data);
  [[nodiscard]]
  nwge::ArrayView<nwge::json::Value> commandsArray() const;
};

} // namespace sigmoid