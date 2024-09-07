#include "Game.hpp"
#include <nwge/dialog.hpp>
#include <nwge/json.hpp>

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
  if(!res.value->isObject()) {
    dialog::error("Failure"_sv,
      "Could not parse GAME.INFO for {}:\n"
      "Expected object.",
      name);
    return false;
  }
  const auto &obj = res.value->object();

  const auto *titleVal = obj.get("title"_sv);
  if(titleVal == nullptr) {
    dialog::warning("Warning",
      "Could not find title in GAME.INFO, fallback will be used.");
    title = name;
  } else {
    if(!titleVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for title.",
        name);
      return false;
    }
    title = titleVal->string();
  }

  const auto *authorVal = obj.get("author"_sv);
  if(authorVal == nullptr) {
    dialog::warning("Warning",
      "Could not find author in GAME.INFO, fallback will be used.");
    author = "Unknown"_sv;
  } else {
    if(!authorVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for author.",
        name);
      return false;
    }
    author = authorVal->string();
  }

  const auto *descriptionVal = obj.get("description"_sv);
  if(descriptionVal == nullptr) {
    dialog::warning("Warning",
      "Could not find description in GAME.INFO, fallback will be used.");
    description = "No description available."_sv;
  } else {
    if(!descriptionVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for description.",
        name);
      return false;
    }
    description = descriptionVal->string();
  }

  const auto *versionVal = obj.get("version"_sv);
  if(versionVal == nullptr) {
    dialog::warning("Warning",
      "Could not find version in GAME.INFO, fallback will be used.");
    version = "0.0.0"_sv;
  } else {
    if(!versionVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for version.",
        name);
      return false;
    }
    version = versionVal->string();
  }

  const auto *logoVal = obj.get("logo"_sv);
  if(logoVal != nullptr) {
    if(!logoVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for logo.",
        name);
      return false;
    }
    logo = logoVal->string();
  }

  const auto *menuBackgroundVal = obj.get("menu_background"_sv);
  if(menuBackgroundVal != nullptr) {
    if(!menuBackgroundVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for menu_background.",
        name);
      return false;
    }
    menuBackground = menuBackgroundVal->string();
  }

  const auto *startSceneVal = obj.get("start_scene"_sv);
  if(startSceneVal == nullptr) {
    dialog::error("Failure",
    "Could not find start_scene in GAME.INFO.");
    return false;
  }
  if(!startSceneVal->isString()) {
    dialog::error("Failure"_sv,
      "Could not parse GAME.INFO for {}:\n"
      "Expected string for start_scene.",
      name);
    return false;
  }
  startScene = startSceneVal->string();

  return true;
}

bool Game::save(data::RW &file) {
  static constexpr usize cPairCount = 7;
  Slice<json::Object::Pair> pairs{cPairCount};
  pairs.push({"title"_sv, title.view()});
  pairs.push({"author"_sv, author.view()});
  pairs.push({"description"_sv, description.view()});
  pairs.push({"version"_sv, version.view()});
  pairs.push({"logo"_sv, logo.view()});
  pairs.push({"menu_background"_sv, menuBackground.view()});
  pairs.push({"start_scene"_sv, startScene.view()});
  auto str = json::encode(json::Object{pairs.view()});
  return file.write(str.view());
}

} // namespace sigmoid
