#include "StoryScene.hpp"
#include <nwge/json/builder.hpp>
#include <nwge/dialog.hpp>

using namespace nwge;

namespace sigmoid {

#define FAIL(...) \
  dialog::error("Failure"_sv, FAIL_HEADER ":\n" __VA_ARGS__); \
  return false;

#define FAIL_IF(cond, ...) \
  if(cond) {\
    FAIL(__VA_ARGS__);\
  }

StringView StoryScene::ensureSprite(const StringView &sprite) {
  for(const auto &oldSprite: sprites) {
    if(oldSprite.view().equals(sprite)) {
      return oldSprite.view();
    }
  }
  sprites.push({sprite});
  return sprites[sprites.size() - 1];
}

StringView StoryScene::ensureActor(const StringView &actor) {
  for(const auto &oldActor: actors) {
    if(oldActor.id.view().equals(actor)) {
      return oldActor.id.view();
    }
  }
  return {};
}

Actor *StoryScene::getActor(const StringView &actor) {
  for(auto &oldActor: actors) {
    if(oldActor.id.view().equals(actor)) {
      return &oldActor;
    }
  }
  return nullptr;
}

const Actor *StoryScene::getActor(const StringView &actor) const {
  for(const auto &oldActor: actors) {
    if(oldActor.id.view().equals(actor)) {
      return &oldActor;
    }
  }
  return nullptr;
}

StringView StoryScene::ensureBackground(const StringView &background) {
  for(const auto &oldBackground: backgrounds) {
    if(oldBackground.view().equals(background)) {
      return oldBackground.view();
    }
  }
  backgrounds.push({background});
  return backgrounds[backgrounds.size() - 1];
}

StringView StoryScene::ensureMusic(const StringView &music) {
  for(const auto &oldMusic: musics) {
    if(oldMusic.view().equals(music)) {
      return oldMusic.view();
    }
  }
  musics.push({music});
  return musics[musics.size() - 1];
}

bool StoryScene::load(json::Schema &root) {
  #define FAIL_HEADER "Could not parse story scene"

  auto maybeActors = root.expectObjectField("actors"_sv);
  FAIL_IF(!maybeActors.present(), "Could not find `actors`.");
  FAIL_IF(!loadActors(*maybeActors), "Could not parse `actors`.");

  auto maybeCommands = root.expectArrayField("commands"_sv);
  FAIL_IF(!maybeCommands.present(), "Could not find `commands`.");
  FAIL_IF(!loadCommands(*maybeCommands), "Could not parse `commands`.");

  return true;

  #undef FAIL_HEADER
}

bool StoryScene::loadActors(json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene actors"

  auto pairs = data.pairs();
  if(pairs.size() == 0) {
    return true;
  }
  actors = {pairs.size()};
  for(const auto &[key, val]: pairs) {
    FAIL_IF(!val.isObject(), "Expected object for actor {}.", key);
    const auto &obj = val.object();
    actors.push({});
    auto &actor = actors[actors.size() - 1];
    FAIL_IF(!actor.load(key, obj), "Could not parse actor object.");
  }
  return true;

  #undef FAIL_HEADER
}

bool Actor::load(const StringView &actorID, const json::Object &data) {
  #define FAIL_HEADER "Could not parse story scene actor {}"

  id = actorID;
  const auto *nameVal = data.get("name"_sv);
  FAIL_IF(nameVal == nullptr, "No `name` field.", actorID);
  FAIL_IF(!nameVal->isString(), "Expected string for `name`.", actorID);
  name = nameVal->string();

  const auto *sheetVal = data.get("sheet"_sv);
  FAIL_IF(sheetVal == nullptr, "No `sheet` field.", actorID);
  FAIL_IF(!sheetVal->isString(), "Expected string for `sheet`.", actorID);
  FAIL_IF(sheetVal->string().empty(),
    "Expected non-empty string for `sheet`.", actorID);
  sheet = sheetVal->string();

  const auto *sheetSizeVal = data.get("sheetSize"_sv);
  FAIL_IF(sheetSizeVal == nullptr, "No `sheetSize` field.", actorID);
  FAIL_IF(!sheetSizeVal->isArray(), "Expected array for `sheetSize`.", actorID);
  auto sizeArr = sheetSizeVal->array();
  FAIL_IF(sizeArr.size() != 2, "Sprite array must be 2 elements.", actorID);
  const auto &xVal = sizeArr[0];
  FAIL_IF(!xVal.isNumber(), "Expected number for sprite array X.", actorID);
  const auto &yVal = sizeArr[1];
  FAIL_IF(!yVal.isNumber(), "Expected number for sprite array Y.", actorID);
  auto width = s32(xVal.number());
  auto height = s32(yVal.number());
  sheetSize = {width, height};

  return true;

  #undef FAIL_HEADER
}

bool StoryScene::loadCommands(json::Schema data) {
  #define FAIL_HEADER "Could not parse story scene commands"

  commands = {data.array().size()};
  for(usize i = 0; i < commands.size(); ++i) {
    auto maybeCommand = data.expectObjectElement();
    FAIL_IF(!maybeCommand.present(), "Could not find command {}.", i);
    auto &cmd = commands[i];

    auto maybeSprite = maybeCommand->expectObjectField("sprite"_sv);
    if(maybeSprite.present()) {
      cmd.code = CommandSprite;
      cmd.sprite.emplace();
      FAIL_IF(!cmd.sprite->load(*this, *maybeSprite),
        "Could not parse sprite command {}.", i);
      continue;
    }

    auto maybeBackground = maybeCommand->expectObjectField("background"_sv);
    if(maybeBackground.present()) {
      cmd.code = CommandBackground;
      cmd.background.emplace();
      FAIL_IF(!cmd.background->load(*this, *maybeBackground),
        "Could not parse background command {}.", i);
      continue;
    }

    auto maybeSpeak = maybeCommand->expectObjectField("speak"_sv);
    if(maybeSpeak.present()) {
      cmd.code = CommandSpeak;
      cmd.speak.emplace();
      FAIL_IF(!cmd.speak->load(*this, *maybeSpeak),
        "Could not parse speak command {}.", i);
      continue;
    }

    auto maybeWait = maybeCommand->expectObjectField("wait"_sv);
    if(maybeWait.present()) {
      cmd.code = CommandWait;
      cmd.wait.emplace();
      FAIL_IF(!cmd.wait->load(*maybeWait),
        "Could not parse wait command {}.", i);
      continue;
    }

    FAIL("Invalid command {}.", i);
    return false;
  }

  return true;

  #undef FAIL_HEADER
}

bool SpriteCommand::load(StoryScene &scene, json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene sprite command"

  auto maybeId = data.expectStringField("id"_sv);
  FAIL_IF(!maybeId.present(), "Could not find id for sprite command.");
  id = scene.ensureSprite(*maybeId);

  auto maybeHide = data.expectBooleanField("hide"_sv);
  if(maybeHide.present()) {
    hide = *maybeHide;
  }

  auto maybeActor = data.expectStringField("actor"_sv);
  FAIL_IF(!maybeActor.present(), "Need actor for sprite command.");
  actor = scene.ensureActor(*maybeActor);
  FAIL_IF(actor.empty(), "Could not find actor {}.", *maybeActor);
  const auto *actorData = scene.getActor(actor);

  auto maybePortrait = data.expectArrayField("portrait"_sv);
  if(maybePortrait.present()) {
    FAIL_IF(!maybeActor.present(), "Need actor for portrait.");
    auto maybeX = maybePortrait->expectNumberElement();
    FAIL_IF(!maybeX.present(), "Could not find x for portrait.");
    auto maybeY = maybePortrait->expectNumberElement();
    FAIL_IF(!maybeY.present(), "Could not find y for portrait.");
    auto portraitX = s32(*maybeX);
    auto portraitY = s32(*maybeY);
    portrait = portraitY * actorData->sheetSize.x + portraitX;
  }

  auto maybePos = data.expectArrayField("pos"_sv);
  if(!maybePos.present()) {
    auto maybeX = maybePos->expectNumberElement();
    FAIL_IF(!maybeX.present(), "Could not find x for pos.");
    auto maybeY = maybePos->expectNumberElement();
    FAIL_IF(!maybeY.present(), "Could not find y for pos.");
    pos = {f32(*maybeX), f32(*maybeY)};
  }

  auto maybeSize = data.expectArrayField("size"_sv);
  if(maybeSize.present()) {
    auto maybeX = maybeSize->expectNumberElement();
    FAIL_IF(!maybeX.present(), "Could not find x for size.");
    auto maybeY = maybeSize->expectNumberElement();
    FAIL_IF(!maybeY.present(), "Could not find y for size.");
    size = {f32(*maybeX), f32(*maybeY)};
  }

  return true;

  #undef FAIL_HEADER
}

bool SpeakCommand::load(struct StoryScene &scene, json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene speak command"

  auto maybeActor = data.expectStringField("actor"_sv);
  FAIL_IF(!maybeActor.present(), "Could not find actor for speak command.");
  actor = scene.ensureActor(*maybeActor);
  FAIL_IF(actor.empty(), "Could not find actor {}.", *maybeActor);
  const auto *actorData = scene.getActor(actor);

  auto maybeText = data.expectStringField("text"_sv);
  if(maybeText.present()) {
    text = *maybeText;
  }

  auto maybePortrait = data.expectArrayField("portrait"_sv);
  if(maybePortrait.present()) {
    auto maybeX = maybePortrait->expectNumberElement();
    FAIL_IF(!maybeX.present(), "Could not find x for portrait.");
    auto maybeY = maybePortrait->expectNumberElement();
    FAIL_IF(!maybeY.present(), "Could not find y for portrait.");
    auto portraitX = s32(*maybeX);
    auto portraitY = s32(*maybeY);
    portrait = portraitY * actorData->sheetSize.x + portraitX;
  }

  return true;

  #undef FAIL_HEADER
}

bool WaitCommand::load(json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene wait command"

  auto maybeTime = data.expectNumberField("time"_sv);
  FAIL_IF(!maybeTime.present(), "Could not find time for wait command.");
  duration = f32(*maybeTime);

  return true;

  #undef FAIL_HEADER
}

bool BackgroundCommand::load(StoryScene &scene, json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene background command"

  auto maybeBackground = data.expectStringField("background"_sv);
  if(maybeBackground.present()) {
    background = scene.ensureBackground(*maybeBackground);
  }

  auto maybeMusic = data.expectStringField("music"_sv);
  if(maybeMusic.present()) {
    music = scene.ensureMusic(*maybeMusic);
  }

  return true;

  #undef FAIL_HEADER
}

json::Object StoryScene::toObject() const {
  Slice<json::Object::Pair> pairs{4};
  pairs.push({"actors"_sv, actorsObject()});
  pairs.push({"commands"_sv, commandsArray()});
  return json::Object{pairs.view()};
}

json::Object StoryScene::actorsObject() const {
  if(actors.size() == 0) {
    return {};
  }
  Slice<json::Object::Pair> pairs{actors.size()};
  for(const auto & actor: actors) {
    pairs.push({actor.id.view(), actor.toObject()});
  }
  return json::Object{pairs.view()};
}

json::Object Actor::toObject() const {
  Slice<json::Object::Pair> pairs{4};
  pairs.push({"name"_sv, name.view()});
  pairs.push({"sheet"_sv, sheet.view()});
  std::array sheetSizeArray{
    json::Value{f64(sheetSize.x)},
    json::Value{f64(sheetSize.y)},
  };
  pairs.push({"sheetSize"_sv, ArrayView(sheetSizeArray.data(), 2)});
  return json::Object{pairs.view()};
}

ArrayView<json::Value> StoryScene::commandsArray() const {
  if(commands.empty()) {
    return {};
  }
  Slice<json::Value> values{commands.size()};
  for(const auto &command: commands) {
    values.push(command.toObject(*this));
  }
  return values.view();
}

json::Object Command::toObject(const StoryScene &scene) const {
  Slice<json::Object::Pair> pairs{1};
  switch(code) {
  case CommandSpeak:
    pairs.push({"speak"_sv, speak->toObject(scene)});
    break;
  case CommandSprite:
    pairs.push({"sprite"_sv, sprite->toObject(scene)});
    break;
  case CommandWait:
    pairs.push({"wait"_sv, wait->toObject()});
    break;
  case CommandBackground:
    pairs.push({"background"_sv, background->toObject()});
    break;
  default:
    NWGE_UNREACHABLE("invalid CommandCode");
  }
  return json::Object{pairs.view()};
}

json::Object SpriteCommand::toObject(const StoryScene &scene) const {
  json::ObjectBuilder builder;
  builder.set("sprite"_sv, id);
  if(!actor.empty()) {
    builder.set("actor"_sv, actor);
    if(portrait >= 0) {
      const auto *actorData = scene.getActor(actor);
      auto portraitX = portrait % actorData->sheetSize.x;
      auto portraitY = portrait / actorData->sheetSize.x;
      builder.array("portrait"_sv)
        .add(f64(portraitX))
        .add(f64(portraitY))
        .end();
    }
  }
  builder.set("hide"_sv, hide);
  if(pos.x != -1 && pos.y != -1) {
    builder.array("pos"_sv)
      .add(f64(pos.x))
      .add(f64(pos.y))
      .end();
  }
  if(size.x != -1 && size.y != -1) {
    builder.array("size"_sv)
      .add(f64(size.x))
      .add(f64(size.y))
      .end();
  }
  return builder.finish();
}

json::Object SpeakCommand::toObject(const StoryScene &scene) const {
  json::ObjectBuilder builder;

  builder.set("actor"_sv, actor);
  if(portrait >= 0) {
    const auto *actorData = scene.getActor(actor);
    auto portraitX = portrait % actorData->sheetSize.x;
    auto portraitY = portrait / actorData->sheetSize.x;
    builder.array("portrait"_sv)
      .add(f64(portraitX))
      .add(f64(portraitY))
      .end();
  }
  builder.set("text"_sv, text.view());
  return builder.finish();
}

json::Object WaitCommand::toObject() const {
  json::ObjectBuilder builder;
  builder.set("time"_sv, duration);
  return builder.finish();
}

json::Object BackgroundCommand::toObject() const {
  json::ObjectBuilder builder;
  if(!background.empty()) {
    builder.set("background"_sv, background);
  }
  if(!music.empty()) {
    builder.set("music"_sv, music);
  }
  return builder.finish();
}

} // namespace sigmoid
