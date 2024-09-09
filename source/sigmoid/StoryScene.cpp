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

bool StoryScene::load(json::Schema &root) {
  #define FAIL_HEADER "Could not parse story scene"

  auto maybeActors = root.expectObjectField("actors"_sv);
  FAIL_IF(!maybeActors.present(), "Could not find `actors`.");
  FAIL_IF(!loadActors(*maybeActors), "Could not parse `actors`.");

  auto maybeCommands = root.expectArrayField("commands"_sv);
  FAIL_IF(!maybeCommands.present(), "Could not find `commands`.");
  FAIL_IF(!loadSprites(*maybeCommands), "Could not parse `commands`.");
  FAIL_IF(!loadBackgrounds(*maybeCommands), "Could not parse `commands`.");
  FAIL_IF(!loadCommands(*maybeCommands), "Could not parse `commands`.");

  return true;

  #undef FAIL_HEADER
}

bool StoryScene::loadActors(json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene actors"

  auto pairs = data.pairs();
  actors = {pairs.size()};
  usize idx = 0;
  for(const auto &[key, val]: pairs) {
    FAIL_IF(!val.isObject(), "Expected object for actor {}.", key);
    const auto &obj = val.object();
    auto &actor = actors[idx++];
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

bool StoryScene::loadSprites(json::Schema data) {
  #define FAIL_HEADER "Could not parse story scene sprites"

  if(!data.hasArrayElement()) {
    return true;
  }

  Slice<const json::Object*> spriteCommands{actors.size()};
  while(data.hasArrayElement()) {
    auto maybeCommand = data.expectObjectElement();
    FAIL_IF(!maybeCommand.present(), "Could not find sprite command.");
    auto maybeSprite = maybeCommand->expectObjectField("sprite"_sv);
    if(!maybeSprite.present()) {
      continue;
    }
    spriteCommands.push(&maybeSprite->object());
  }

  Slice<Sprite> spriteData{spriteCommands.size()};
  for(const auto *command: spriteCommands) {
    const auto *idVal = command->get("id"_sv);
    FAIL_IF(idVal == nullptr, "Could not find id for sprite command.");
    FAIL_IF(!idVal->isString(), "Expected string for sprite command id.");
    const auto &spriteID = idVal->string();
    for(auto &sprite: spriteData) {
      if(sprite.id.view() == spriteID) {
        goto next;
      }
    }
    spriteData.push({spriteID});
    next:;
  }

  sprites = spriteData.view();
  return true;

  #undef FAIL_HEADER
}

bool StoryScene::loadBackgrounds(json::Schema data) {
  #define FAIL_HEADER "Could not parse story scene backgrounds"

  if(!data.hasArrayElement()) {
    return true;
  }

  Slice<const json::Object*> backgroundCommands{actors.size()};
  while(data.hasArrayElement()) {
    auto maybeCommand = data.expectObjectElement();
    FAIL_IF(!maybeCommand.present(), "Could not find background command.");
    auto maybeBackground = maybeCommand->expectObjectField("background"_sv);
    if(!maybeBackground.present()) {
      continue;
    }
    backgroundCommands.push(&maybeBackground->object());
  }

  Slice<StringView> backgroundNames{backgroundCommands.size()};
  Slice<StringView> musicTrackNames{backgroundCommands.size()};
  for(const auto *command: backgroundCommands) {
    const auto *imageVal = command->get("image"_sv);
    if(imageVal != nullptr) {
      FAIL_IF(!imageVal->isString(), "Expected string for background image.");
      for(auto &name: backgroundNames) {
        if(name == imageVal->string()) {
          goto tryMusic;
        }
      }
      backgroundNames.push(imageVal->string());
    }
    tryMusic:
    const auto *musicVal = command->get("music"_sv);
    if(musicVal != nullptr) {
      FAIL_IF(!musicVal->isString(), "Expected string for background music.");
      for(auto &name: musicTrackNames) {
        if(name == musicVal->string()) {
          goto next;
        }
      }
      musicTrackNames.push(musicVal->string());
    }
    next:;
  }

  bgImages = {backgroundNames.size()};
  for(usize i = 0; i < bgImages.size(); ++i) {
    bgImages[i] = {backgroundNames[i]};
  }
  musicTracks = {musicTrackNames.size()};
  for(usize i = 0; i < musicTracks.size(); ++i) {
    musicTracks[i] = {musicTrackNames[i]};
  }
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

bool SpriteCommand::load(const StoryScene &scene, json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene sprite command"

  auto maybeId = data.expectStringField("id"_sv);
  FAIL_IF(!maybeId.present(), "Could not find id for sprite command.");

  const auto &spriteID = *maybeId;
  usize sprite = 0;
  for(; sprite < scene.sprites.size(); ++sprite) {
    if(scene.sprites[sprite].id.view() == spriteID) {
      break;
    }
  }
  FAIL_IF(sprite == scene.sprites.size(),
    "Could not find sprite with id {}.", spriteID);

  auto maybeHide = data.expectBooleanField("hide"_sv);
  if(maybeHide.present()) {
    hide = *maybeHide;
  }

  auto maybeActor = data.expectStringField("actor"_sv);
  if(maybeActor.present()) {
    actor = 0;
    for(; actor < scene.actors.size(); ++actor) {
      if(scene.actors[actor].id.view() == *maybeActor) {
        break;
      }
    }
    FAIL_IF(actor == scene.actors.size(),
      "Could not find actor with id {}.", maybeActor);
  }

  auto maybePortrait = data.expectArrayField("portrait"_sv);
  if(maybePortrait.present()) {
    FAIL_IF(!maybeActor.present(), "Need actor for portrait.");
    auto maybeX = maybePortrait->expectNumberElement();
    FAIL_IF(!maybeX.present(), "Could not find x for portrait.");
    auto maybeY = maybePortrait->expectNumberElement();
    FAIL_IF(!maybeY.present(), "Could not find y for portrait.");
    auto portraitX = s32(*maybeX);
    auto portraitY = s32(*maybeY);
    const auto &actorData = scene.actors[actor];
    portrait = portraitY * actorData.sheetSize.x + portraitX;
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

bool SpeakCommand::load(const struct StoryScene &scene, json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene speak command"

  auto maybeActor = data.expectStringField("actor"_sv);
  FAIL_IF(!maybeActor.present(), "Could not find actor for speak command.");
  actor = 0;
  for(; actor < scene.actors.size(); ++actor) {
    if(scene.actors[actor].id.view() == *maybeActor) {
      break;
    }
  }
  FAIL_IF(actor == scene.actors.size(), "Could not find actor with id {}.",
    *maybeActor);

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
    const auto &actorData = scene.actors[actor];
    portrait = portraitY * actorData.sheetSize.x + portraitX;
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

bool BackgroundCommand::load(const StoryScene &scene, json::Schema &data) {
  #define FAIL_HEADER "Could not parse story scene background command"

  auto maybeImage = data.expectStringField("image"_sv);
  FAIL_IF(!maybeImage.present(), "Could not find image for background command.");
  const auto &imageName = *maybeImage;
  for(image = 0; image < scene.bgImages.size(); ++image) {
    if(scene.bgImages[image].view() == imageName) {
      break;
    }
  }
  FAIL_IF(image == scene.bgImages.size(), "Could not find background with id {}.", imageName);

  auto maybeMusic = data.expectStringField("music"_sv);
  if(maybeMusic.present()) {
    const auto &musicName = *maybeMusic;
    for(music = 0; music < scene.musicTracks.size(); ++music) {
      if(scene.musicTracks[music].view() == musicName) {
        break;
      }
    }
    FAIL_IF(music == scene.musicTracks.size(), "Could not find music with id {}.", musicName);
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
  if(actors.empty()) {
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
    pairs.push({"background"_sv, background->toObject(scene)});
    break;
  default:
    NWGE_UNREACHABLE("invalid CommandCode");
  }
  return json::Object{pairs.view()};
}

json::Object SpriteCommand::toObject(const StoryScene &scene) const {
  json::ObjectBuilder builder;
  builder.set("sprite"_sv, scene.sprites[sprite].id.view());
  if(actor >= 0) {
    builder.set("actor"_sv, scene.actors[actor].id.view());
    if(portrait >= 0) {
      auto portraitX = portrait % scene.actors[actor].sheetSize.x;
      auto portraitY = portrait / scene.actors[actor].sheetSize.x;
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

  builder.set("actor"_sv, scene.actors[actor].id.view());
  if(portrait >= 0) {
    auto portraitX = portrait % scene.actors[actor].sheetSize.x;
    auto portraitY = portrait / scene.actors[actor].sheetSize.x;
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

json::Object BackgroundCommand::toObject(const StoryScene &scene) const {
  json::ObjectBuilder builder;
  if(image >= 0) {
    builder.set("image"_sv, scene.bgImages[image].view());
  }
  if(music >= 0) {
    builder.set("music"_sv, scene.musicTracks[music].view());
  }
  return builder.finish();
}

} // namespace sigmoid
