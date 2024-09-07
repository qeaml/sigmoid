#include "Scene.hpp"
#include <nwge/bndl/tree.h>
#include <nwge/dialog.hpp>

using namespace nwge;

namespace sigmoid {

Scene::Scene(const nwge::StringView &name)
  : name(name)
{}

Scene::~Scene() = default;

void Scene::enqueue(data::Bundle &bundle) {
  ScratchArray<char> filename = ScratchString::formatted("{}.scn", name);
  toUpper(filename.view());
  bundle.nqCustom(filename.view(), *this);
}

bool Scene::load(data::RW &file) {
  s64 size = file.size();
  if(size <= 0) {
    dialog::error("Failure"_sv,
      "Could not load scene {}.",
      name);
    return false;
  }

  ScratchArray<char> raw{usize(size)};
  if(!file.read(raw.view())) {
    dialog::error("Failure"_sv,
      "Could not load scene {}.",
      name);
    return false;
  }

  auto res = json::parse(raw.view());
  if(res.error != json::OK) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "{}",
      name,
      json::errorMessage(res.error));
    return false;
  }
  if(!res.value->isObject()) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Expected object.",
      name);
    return false;
  }
  const auto &obj = res.value->object();

  const auto *titleVal = obj.get("title"_sv);
  if(titleVal == nullptr) {
    dialog::warning("Warning",
      "Could not find title in scene {}, fallback will be used.",
      name);
    title = name;
  } else {
    if(!titleVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for title.",
        name);
      return false;
    }
    title = titleVal->string();
  }

  const auto *backgroundVal = obj.get("background"_sv);
  if(backgroundVal == nullptr) {
    dialog::warning("Warning",
      "Could not find background in scene {}, fallback will be used.",
      name);
  } else {
    if(!backgroundVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for background.",
        name);
      return false;
    }
    background = backgroundVal->string();
  }

  const auto *musicVal = obj.get("music"_sv);
  if(musicVal == nullptr) {
    dialog::warning("Warning",
      "Could not find music in scene {}, fallback will be used.",
      name);
  } else {
    if(!musicVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for music.",
        name);
      return false;
    }
    music = musicVal->string();
  }

  const auto *nextVal = obj.get("next"_sv);
  if(nextVal == nullptr) {
    dialog::warning("Warning",
      "Could not find next in scene {}, fallback will be used.",
      name);
  } else {
    if(!nextVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse scene {}:\n"
        "Expected string for next.",
        name);
      return false;
    }
    next = nextVal->string();
  }

  const auto *typeVal = obj.get("type"_sv);
  if(typeVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Could not find type.",
      name);
    return false;
  }
  if(!typeVal->isString()) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Expected string for type.",
      name);
    return false;
  }

  const auto &typeName = typeVal->string();
  if(typeName.equalsIgnoreCase("field"_sv)) {
    type = SceneField;
    // TODO: FieldScene loading
  } else if(typeName.equalsIgnoreCase("story"_sv)) {
    type = SceneStory;
    story.emplace();
    if(!story->load(obj)) {
      return false;
    }
  } else {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Unknown scene type {}.",
      name, typeName);
    return false;
  }
  return true;
}

bool Scene::save(data::RW &file) {
  ScratchSlice<json::Object::Pair> pairs{4};
  if(!title.empty()) {
    pairs.push({"title"_sv, title.view()});
  }
  if(!background.empty()) {
    pairs.push({"background"_sv, background.view()});
  }
  if(!music.empty()) {
    pairs.push({"music"_sv, music.view()});
  }
  if(!next.empty()) {
    pairs.push({"next"_sv, next.view()});
  }
  switch(type) {
  case SceneField:
    pairs.push({"type"_sv, "field"_sv});
    break;
  case SceneStory: {
    pairs.push({"type"_sv, "story"_sv});
    auto object = story->toObject();
    for(const auto &pair: object.pairs()) {
      pairs.push(pair);
    }
    break;
  }
  case SceneInvalid:
  case SceneMax:
    return false;
  }
  json::Object object{pairs.view()};
  auto res = json::encode(object);
  return file.write(res.view());
}

} // namespace sigmoid