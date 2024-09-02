#include "Game.hpp"
#include <nwge/dialog.hpp>
#include <nwge/json.hpp>

using namespace nwge;

namespace sigmoid {

Game::Game(const StringView &name)
  : mName(name)
{}

Game::~Game() = default;

void Game::preload() {
  ScratchString filename = ScratchString::formatted("{}.bndl", mName);
  mBundle
    .load({"games"_sv, filename})
    .nqCustom("GAME.INFO"_sv, *this);
}

bool Game::load(data::RW &file) {
  s64 size = file.size();
  if(size <= 0) {
    dialog::error("Failure"_sv,
      "Could not load GAME.INFO for {}.",
      mName);
    return false;
  }

  ScratchArray<char> raw{usize(size)};
  if(!file.read(raw.view())) {
    dialog::error("Failure"_sv,
      "Could not load GAME.INFO for {}.",
      mName);
    return false;
  }

  auto res = json::parse(raw.view());
  if(res.error != json::OK) {
    dialog::error("Failure"_sv,
      "Could not parse GAME.INFO for {}:\n"
      "{}",
      mName,
      json::errorMessage(res.error));
    return false;
  }
  if(!res.value->isObject()) {
    dialog::error("Failure"_sv,
      "Could not parse GAME.INFO for {}:\n"
      "Expected object.",
      mName);
    return false;
  }
  const auto &obj = res.value->object();

  const auto *titleVal = obj.get("title"_sv);
  if(titleVal == nullptr) {
    dialog::warning("Warning",
      "Could not find title in GAME.INFO, fallback will be used.");
    mTitle = mName;
  } else {
    if(!titleVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for title.",
        mName);
      return false;
    }
    mTitle = titleVal->string();
  }

  const auto *authorVal = obj.get("author"_sv);
  if(authorVal == nullptr) {
    dialog::warning("Warning",
      "Could not find author in GAME.INFO, fallback will be used.");
    mAuthor = "Unknown"_sv;
  } else {
    if(!authorVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for author.",
        mName);
      return false;
    }
    mAuthor = authorVal->string();
  }

  const auto *descriptionVal = obj.get("description"_sv);
  if(descriptionVal == nullptr) {
    dialog::warning("Warning",
      "Could not find description in GAME.INFO, fallback will be used.");
    mDescription = "No description available."_sv;
  } else {
    if(!descriptionVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for description.",
        mName);
      return false;
    }
    mDescription = descriptionVal->string();
  }

  const auto *versionVal = obj.get("version"_sv);
  if(versionVal == nullptr) {
    dialog::warning("Warning",
      "Could not find version in GAME.INFO, fallback will be used.");
    mVersion = "0.0.0"_sv;
  } else {
    if(!versionVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for version.",
        mName);
      return false;
    }
    mVersion = versionVal->string();
  }

  const auto *logoVal = obj.get("logo"_sv);
  if(logoVal != nullptr) {
    if(!logoVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for logo.",
        mName);
      return false;
    }
    mLogo = logoVal->string();
  }

  const auto *menuBackgroundVal = obj.get("menu_background"_sv);
  if(menuBackgroundVal != nullptr) {
    if(!menuBackgroundVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse GAME.INFO for {}:\n"
        "Expected string for menu_background.",
        mName);
      return false;
    }
    mMenuBackground = menuBackgroundVal->string();
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
      mName);
    return false;
  }
  mStartScene = startSceneVal->string();

  return true;
}

} // namespace sigmoid
