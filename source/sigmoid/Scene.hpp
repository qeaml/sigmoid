#pragma once

/*
Scene.hpp
---------
Scene definition
*/

#include "StoryScene.hpp"
#include <nwge/common/slice.hpp>
#include <nwge/common/string.hpp>
#include <nwge/data/bundle.hpp>
#include <nwge/json.hpp>

namespace sigmoid {

enum SceneType {
  SceneInvalid = -1,
  SceneField,
  SceneStory,
  SceneMax
};

class Scene {
public:
  nwge::String<> name;
  nwge::String<> title;
  nwge::String<> background;
  nwge::String<> music;
  nwge::String<> next;
  SceneType type = SceneInvalid;
  nwge::Maybe<StoryScene> story;

  Scene(const nwge::StringView &name);
  Scene(Scene&&) = default;
  Scene(const Scene&) = delete;
  Scene& operator=(Scene&&) = default;
  Scene& operator=(const Scene&) = delete;
  ~Scene();

  void enqueue(nwge::data::Bundle &bundle);
  bool load(nwge::data::RW &file);
  bool save(nwge::data::RW &file);
};

} // namespace sigmoid