#pragma once

/*
Game.hpp
--------
Defines game information
*/

#include "Scene.hpp"
#include <nwge/common/string.hpp>
#include <nwge/data/bundle.hpp>

namespace sigmoid {

class Game {
public:
  Game(const nwge::StringView &name);
  Game(Game&&) = default;
  Game(const Game&) = delete;
  Game& operator=(Game&&) = default;
  Game& operator=(const Game&) = delete;
  ~Game();

  [[nodiscard]]
  inline nwge::StringView title() const {
    return mTitle;
  }

  [[nodiscard]]
  inline nwge::StringView author() const {
    return mAuthor;
  }

  [[nodiscard]]
  inline nwge::StringView description() const {
    return mDescription;
  }

  [[nodiscard]]
  inline nwge::StringView version() const {
    return mVersion;
  }

  [[nodiscard]]
  inline nwge::StringView logo() const {
    return mLogo;
  }

  [[nodiscard]]
  inline nwge::StringView menuBackground() const {
    return mMenuBackground;
  }

  [[nodiscard]]
  inline nwge::StringView startScene() const {
    return mStartScene;
  }

  [[nodiscard]]
  inline nwge::data::Bundle &bundle() {
    return mBundle;
  }

  void preload(); // -> enqueue data to be loaded
  bool load(nwge::data::RW &file);

private:
  nwge::String<> mName;        // -> name of bundle file
  nwge::String<> mTitle;       // -> title of game shown in menu
  nwge::String<> mAuthor;
  nwge::String<> mDescription;
  nwge::String<> mVersion;
  nwge::String<> mLogo;           // -> filename of logo graphic, can be empty
  nwge::String<> mMenuBackground; // -> menu background graphic, can be empty
  nwge::String<> mStartScene;
  nwge::data::Bundle mBundle;
};

} // namespace sigmoid
