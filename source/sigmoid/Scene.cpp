#include "Scene.hpp"
#include <nwge/bndl/tree.h>
#include <nwge/dialog.hpp>

using namespace nwge;

namespace sigmoid {

Scene::Scene(const nwge::StringView &name)
  : mName(name)
{}

Scene::~Scene() = default;

void Scene::enqueue(data::Bundle &bundle) {
  ScratchArray<char> filename = ScratchString::formatted("{}.scn", mName);
  toUpper(filename.view());
  bundle.nqCustom(filename.view(), *this);
}

bool Scene::load(data::RW &file) {
  s64 size = file.size();
  if(size <= 0) {
    dialog::error("Failure"_sv,
      "Could not load scene {}.",
      mName);
    return false;
  }

  ScratchArray<char> raw{usize(size)};
  if(!file.read(raw.view())) {
    dialog::error("Failure"_sv,
      "Could not load scene {}.",
      mName);
    return false;
  }

  auto res = json::parse(raw.view());
  if(res.error != json::OK) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "{}",
      mName,
      json::errorMessage(res.error));
    return false;
  }
  if(!res.value->isObject()) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Expected object.",
      mName);
    return false;
  }
  const auto &obj = res.value->object();

  const auto *titleVal = obj.get("title"_sv);
  if(titleVal == nullptr) {
    dialog::warning("Warning",
      "Could not find title in scene {}, fallback will be used.",
      mName);
    mTitle = mName;
  } else {
    if(!titleVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for title.",
        mName);
      return false;
    }
    mTitle = titleVal->string();
  }

  const auto *backgroundVal = obj.get("background"_sv);
  if(backgroundVal == nullptr) {
    dialog::warning("Warning",
      "Could not find background in scene {}, fallback will be used.",
      mName);
  } else {
    if(!backgroundVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for background.",
        mName);
      return false;
    }
    mBackground = backgroundVal->string();
  }

  const auto *musicVal = obj.get("music"_sv);
  if(musicVal == nullptr) {
    dialog::warning("Warning",
      "Could not find music in scene {}, fallback will be used.",
      mName);
  } else {
    if(!musicVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for music.",
        mName);
      return false;
    }
    mMusic = musicVal->string();
  }

  const auto *nextVal = obj.get("next"_sv);
  if(nextVal == nullptr) {
    dialog::warning("Warning",
      "Could not find next in scene {}, fallback will be used.",
      mName);
  } else {
    if(!nextVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for next.",
        mName);
      return false;
    }
  }

  const auto *typeVal = obj.get("type"_sv);
  if(typeVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Could not find type.",
      mName);
    return false;
  }
  if(!typeVal->isString()) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Expected string for type.",
      mName);
    return false;
  }

  const auto &type = typeVal->string();
  if(type.equalsIgnoreCase("field"_sv)) {
    mType = SceneField;
    // TODO: FieldScene loading
  } else if(type.equalsIgnoreCase("story"_sv)) {
    mType = SceneStory;
    mStory.emplace();
    if(!mStory->load(obj)) {
      return false;
    }
  } else {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Unknown scene type {}.",
      mName, type);
    return false;
  }
  return true;
}

} // namespace sigmoid