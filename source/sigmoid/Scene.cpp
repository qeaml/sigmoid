#include "Scene.hpp"
#include <nwge/bndl/tree.h>
#include <nwge/dialog.hpp>
#include <nwge/json/Schema.hpp>

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

  auto maybeRoot = json::Schema::object(*res.value);
  if(!maybeRoot.present()) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}\n"
      "Expected object.",
      name);
    return false;
  }

  auto maybeTitle = maybeRoot->expectStringField("title"_sv);
  if(!maybeTitle.present()) {
    dialog::warning("Warning",
      "Could not find title in scene {}, fallback will be used.",
      name);
    title = name;
  } else {
    title = *maybeTitle;
  }

  auto maybeBackground = maybeRoot->expectStringField("background"_sv);
  if(maybeBackground.present()) {
    background = *maybeBackground;
  }

  auto maybeMusic = maybeRoot->expectStringField("music"_sv);
  if(maybeMusic.present()) {
    music = *maybeMusic;
  }

  auto maybeNext =  maybeRoot->expectStringField("next"_sv);
  if(maybeNext.present()) {
    next = *maybeNext;
  }

  auto maybeType = maybeRoot->expectStringField("type"_sv);
  if(!maybeType.present()) {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Could not find type.",
      name);
    return false;
  }

  if(maybeType->equalsIgnoreCase("field"_sv)) {
    type = SceneField;
    // TODO: FieldScene loading
  } else if(maybeType->equalsIgnoreCase("story"_sv)) {
    type = SceneStory;
    story.emplace();
    if(!story->load(*maybeRoot)) {
      return false;
    }
  } else {
    dialog::error("Failure"_sv,
      "Could not parse scene {}:\n"
      "Unknown scene type {}.",
      name, maybeType);
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