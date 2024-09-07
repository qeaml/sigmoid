#pragma once

/*
Game.hpp
--------
Defines game information
*/

#include <nwge/common/string.hpp>
#include <nwge/data/bundle.hpp>

namespace sigmoid {

class Game {
public:
  nwge::data::Bundle bundle;
  nwge::String<> name;        // -> name of bundle file
  nwge::String<> title;       // -> title of game shown in menu
  nwge::String<> author;
  nwge::String<> description;
  nwge::String<> version;
  nwge::String<> logo;           // -> filename of logo graphic, can be empty
  nwge::String<> menuBackground; // -> menu background graphic, can be empty
  nwge::String<> startScene;

  Game(const nwge::StringView &name);
  Game(Game&&) = default;
  Game(const Game&) = delete;
  Game& operator=(Game&&) = default;
  Game& operator=(const Game&) = delete;
  ~Game();

  void preload(); // -> enqueue data to be loaded
  bool load(nwge::data::RW &file);
  bool save(nwge::data::RW &file);
};

} // namespace sigmoid
