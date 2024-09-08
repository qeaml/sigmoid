#include "Game.hpp"
#include <nwge/dialog.hpp>
#include <nwge/json.hpp>
#include <nwge/json/builder.hpp>
#include <nwge/json/Schema.hpp>

using namespace nwge;

namespace sigmoid {

Game::Game(const StringView &name)
  : name(name)
{}

Game::~Game() = default;

void Game::preload() {
  ScratchString filename = ScratchString::formatted("{}.bndl", name);
  bundle
    .load({"games"_sv, filename})
    .nqCustom("GAME.INFO"_sv, *this);
}

bool Game::load(data::RW &file) {
  s64 size = file.size();
  if(size <= 0) {
    dialog::error("Failure"_sv,
      "Could not load GAME.INFO for {}.",
      name);
    return false;
  }

  ScratchArray<char> raw{usize(size)};
  if(!file.read(raw.view())) {
    dialog::error("Failure"_sv,
      "Could not load GAME.INFO for {}.",
      name);
    return false;
  }

  auto res = json::parse(raw.view());
  if(res.error != json::OK) {
    dialog::error("Failure"_sv,
      "Could not parse GAME.INFO for {}:\n"
      "{}",
      name,
      json::errorMessage(res.error));
    return false;
  }


  auto root = json::Schema::object(*res.value);
  if(!root.present()) {
    dialog::error("Failure"_sv,
      "Could not parse GAME.INFO for {}:\n"
      "Expected object.",
      name);
    return false;
  }

  auto maybeTitle = root->expectStringField("title"_sv);
  if(!maybeTitle.present()) {
    dialog::warning("Warning",
      "Could not find title in GAME.INFO, fallback will be used.");
    title = name;
  } else {
    title = *maybeTitle;
  }

  auto maybeAuthor = root->expectStringField("author"_sv);
  if(!maybeAuthor.present()) {
    dialog::warning("Warning",
      "Could not find author in GAME.INFO, fallback will be used.");
    author = "Unknown"_sv;
  } else {
    author = *maybeAuthor;
  }

  auto maybeDescription = root->expectStringField("description"_sv);
  if(!maybeDescription.present()) {
    dialog::warning("Warning",
      "Could not find description in GAME.INFO, fallback will be used.");
    description = "No description available."_sv;
  } else {
    description = *maybeDescription;
  }

  auto maybeVersion = root->expectStringField("version"_sv);
  if(!maybeVersion.present()) {
    dialog::warning("Warning",
      "Could not find version in GAME.INFO, fallback will be used.");
    version = "0.0.0"_sv;
  } else {
    version = *maybeVersion;
  }
  auto maybeLogo = root->expectStringField("logo"_sv);
  if(maybeLogo.present()) {
    logo = *maybeLogo;
  }

  auto maybeBackground = root->expectStringField("menu_background"_sv);
  if(!maybeBackground.present()) {
    dialog::warning("Warning",
      "Could not find background in GAME.INFO, fallback will be used.");
  } else {
    menuBackground = *maybeBackground;
  }

  auto maybeStartScene = root->expectStringField("start_scene"_sv);
  if(!maybeStartScene.present()) {
    dialog::error("Failure",
    "Could not find start_scene in GAME.INFO.");
    return false;
  }
  startScene = *maybeStartScene;

  return true;
}

bool Game::save(data::RW &file) {
  json::ObjectBuilder builder;
  builder.set("title"_sv, title.view());
  builder.set("author"_sv, author.view());
  builder.set("description"_sv, description.view());
  builder.set("version"_sv, version.view());
  builder.set("logo"_sv, logo.view());
  builder.set("menu_background"_sv, menuBackground.view());
  builder.set("start_scene"_sv, startScene.view());
  auto str = json::encode(builder.finish());
  return file.write(str.view());
}

} // namespace sigmoid
