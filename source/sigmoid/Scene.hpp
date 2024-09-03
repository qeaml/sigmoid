#pragma once

/*
Scene.hpp
---------
Scene definition
*/

#include "StoryScene.hpp"
#include <nwge/common/string.hpp>
#include <nwge/data/bundle.hpp>

namespace sigmoid {

enum SceneType {
  SceneInvalid = -1,
  SceneField,
  SceneStory,
  SceneMax
};

class Scene {
public:
  Scene(const nwge::StringView &name);
  Scene(Scene&&) = default;
  Scene(const Scene&) = delete;
  Scene& operator=(Scene&&) = default;
  Scene& operator=(const Scene&) = delete;
  ~Scene();

  [[nodiscard]]
  inline nwge::StringView name() const {
    return mName;
  }

  [[nodiscard]]
  inline nwge::StringView title() const {
    return mTitle;
  }

  [[nodiscard]]
  inline nwge::StringView background() const {
    return mBackground;
  }

  [[nodiscard]]
  inline nwge::StringView music() const {
    return mMusic;
  }

  [[nodiscard]]
  inline nwge::StringView next() const {
    return mNext;
  }

  [[nodiscard]]
  inline SceneType type() const {
    return mType;
  }

  [[nodiscard]]
  inline const StoryScene &story() const {
    return *mStory;
  }

  void enqueue(nwge::data::Bundle &bundle);
  bool load(nwge::data::RW &file);

private:
  nwge::String<> mName;
  nwge::String<> mTitle;
  nwge::String<> mBackground;
  nwge::String<> mMusic;
  nwge::String<> mNext;
  SceneType mType = SceneInvalid;
  nwge::Maybe<StoryScene> mStory;
};

} // namespace sigmoid