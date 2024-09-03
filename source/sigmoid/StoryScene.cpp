#include "StoryScene.hpp"
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

bool StoryScene::load(const json::Object &data) {
  #define FAIL_HEADER "Could not parse story scene"

  const auto *actorsVal = data.get("actors"_sv);
  FAIL_IF(actorsVal == nullptr, "No `actors` field.");
  FAIL_IF(!actorsVal->isObject(), "Expected object for actors.");
  FAIL_IF(!loadActors(actorsVal->object()), "Could not parse `actors`.");

  const auto *commandsVal = data.get("commands"_sv);
  FAIL_IF(commandsVal == nullptr, "No `commands` field.");
  FAIL_IF(!commandsVal->isArray(), "Expected array for commands.");
  FAIL_IF(!loadSprites(commandsVal->array()), "Could not parse `commands`.");
  FAIL_IF(!loadBackgrounds(commandsVal->array()), "Could not parse `commands`.");
  FAIL_IF(!loadCommands(commandsVal->array()), "Could not parse `commands`.");

  return true;

  #undef FAIL_HEADER
}

bool StoryScene::loadActors(const json::Object &data) {
  #define FAIL_HEADER "Could not parse story scene actors"

  actors = {data.size()};
  usize idx = 0;
  for(const auto &[key, val]: data.pairs()) {
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

  const auto *spritesVal = data.get("sprites"_sv);
  FAIL_IF(spritesVal == nullptr, "No `sprites` field.", actorID);
  FAIL_IF(!spritesVal->isArray(), "Expected array for `sprites`.", actorID);
  auto spriteArr = spritesVal->array();
  FAIL_IF(spriteArr.size() != 2, "Sprite array must be 2 elements.", actorID);
  const auto &xVal = spriteArr[0];
  FAIL_IF(!xVal.isNumber(), "Expected number for sprite array X.", actorID);
  const auto &yVal = spriteArr[1];
  FAIL_IF(!yVal.isNumber(), "Expected number for sprite array Y.", actorID);
  auto width = s32(xVal.number());
  auto height = s32(yVal.number());
  sheetSize = {width, height};
  sprites = {usize(width * height)};
  for(usize i = 0; i < sprites.size(); ++i) {
    sprites[i] = render::TexCoord{
      {f32(i % width) / f32(width), f32(usize(i / width)) / f32(height)},
      {1.0f / f32(width), 1.0f / f32(height)},
    };
  }

  return true;

  #undef FAIL_HEADER
}

bool StoryScene::loadSprites(const ArrayView<json::Value> &commandData) {
  #define FAIL_HEADER "Could not parse story scene sprites"

  Slice<const json::Object*> spriteCommands{actors.size()};
  for(const auto &val: commandData) {
    FAIL_IF(!val.isObject(), "Expected object for command.");
    const auto &obj = val.object();
    const auto *spriteVal = obj.get("sprite"_sv);
    if(spriteVal == nullptr) {
      continue;
    }
    FAIL_IF(!spriteVal->isObject(), "Expected object for sprite command.");
    const auto &spriteObj = spriteVal->object();
    spriteCommands.push(&spriteObj);
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

bool StoryScene::loadBackgrounds(const ArrayView<json::Value> &commandData) {
  #define FAIL_HEADER "Could not parse story scene backgrounds"

  Slice<const json::Object*> backgroundCommands{actors.size()};
  for(const auto &val: commandData) {
    FAIL_IF(!val.isObject(), "Expected object for command.");
    const auto &obj = val.object();
    const auto *backgroundVal = obj.get("background"_sv);
    if(backgroundVal == nullptr) {
      continue;
    }
    FAIL_IF(!backgroundVal->isObject(), "Expected object for background command.");
    const auto &spriteObj = backgroundVal->object();
    backgroundCommands.push(&spriteObj);
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

bool StoryScene::loadCommands(const ArrayView<json::Value> &data) {
  #define FAIL_HEADER "Could not parse story scene commands"

  commands = {data.size()};
  for(usize i = 0; i < data.size(); ++i) {
    const auto &command = data[i];
    FAIL_IF(!command.isObject(), "Expected object for command {}.", i);
    auto &cmd = commands[i];
    const auto &obj = command.object();

    const auto *data = obj.get("sprite"_sv);
    if(data != nullptr) {
      FAIL_IF(!data->isObject(), "Expected object for sprite command {}.", i);
      cmd.code = CommandSprite;
      cmd.sprite.emplace();
      FAIL_IF(!cmd.sprite->load(*this, data->object()),
        "Could not parse sprite command {}.", i);
      continue;
    }

    data = obj.get("speak"_sv);
    if(data != nullptr) {
      FAIL_IF(!data->isObject(), "Expected object for speak command {}.", i);
      cmd.code = CommandSpeak;
      cmd.speak.emplace();
      FAIL_IF(!cmd.speak->load(*this, data->object()),
        "Could not parse speak command {}.", i);
      continue;
    }

    data = obj.get("wait"_sv);
    if(data != nullptr) {
      FAIL_IF(!data->isObject(), "Expected object for wait command {}.", i);
      cmd.code = CommandWait;
      cmd.wait.emplace();
      FAIL_IF(!cmd.wait->load(data->object()),
        "Could not parse wait command {}.", i);
      continue;
    }

    data = obj.get("background"_sv);
    if(data != nullptr) {
      FAIL_IF(!data->isObject(), "Expected object for background command {}.", i);
      cmd.code = CommandBackground;
      cmd.background.emplace();
      FAIL_IF(!cmd.background->load(*this, data->object()),
        "Could not parse background command {}.", i);
      continue;
    }

    FAIL("Invalid command {}.", i);
    return false;
  }

  return true;

  #undef FAIL_HEADER
}

bool SpriteCommand::load(const StoryScene &scene, const json::Object &data) {
  #define FAIL_HEADER "Could not parse story scene sprite command"

  const auto *idVal = data.get("id"_sv);
  FAIL_IF(idVal == nullptr, "Could not find id for sprite command.");
  FAIL_IF(!idVal->isString(), "Expected string for sprite command id.");
  const auto &spriteID = idVal->string();
  usize sprite = 0;
  for(; sprite < scene.sprites.size(); ++sprite) {
    if(scene.sprites[sprite].id.view() == spriteID) {
      break;
    }
  }
  FAIL_IF(sprite == scene.sprites.size(),
    "Could not find sprite with id {}.", spriteID);

  const auto *hideVal = data.get("hide"_sv);
  if(hideVal != nullptr) {
    FAIL_IF(!hideVal->isBoolean(), "Expected bool for hide.");
    hide = hideVal->boolean();
  }

  const auto *actorVal = data.get("actor"_sv);
  if(actorVal != nullptr) {
    FAIL_IF(!actorVal->isString(), "Expected string for actor.");
    actor = 0;
    for(; actor < scene.actors.size(); ++actor) {
      if(scene.actors[actor].id.view() == actorVal->string()) {
        break;
      }
    }
    FAIL_IF(actor == scene.actors.size(),
      "Could not find actor with id {}.", actorVal->string());
  }

  const auto *portraitVal = data.get("portrait"_sv);
  if(portraitVal != nullptr) {
    FAIL_IF(actorVal == nullptr, "Need actor for portrait.");
    FAIL_IF(!portraitVal->isArray(), "Expected array for portrait.");
    auto portraitArr = portraitVal->array();
    FAIL_IF(portraitArr.size() != 2, "Expected array of size 2 for portrait.");
    auto portraitX = s32(portraitArr[0].number());
    auto portraitY = s32(portraitArr[1].number());
    const auto &actorData = scene.actors[actor];
    portrait = portraitY * actorData.sheetSize.x + portraitX;
  }

  const auto *posVal = data.get("pos"_sv);
  if(posVal != nullptr) {
    FAIL_IF(!posVal->isArray(), "Expected array for pos.");
    auto posArr = posVal->array();
    FAIL_IF(posArr.size() != 2, "Expected array of size 2 for pos.");
    pos = {f32(posArr[0].number()), f32(posArr[1].number())};
  }

  const auto *sizeVal = data.get("size"_sv);
  if(sizeVal != nullptr) {
    FAIL_IF(!sizeVal->isArray(), "Expected array for size.");
    auto sizeArr = sizeVal->array();
    FAIL_IF(sizeArr.size() != 2, "Expected array of size 2 for size.");
    size = {f32(sizeArr[0].number()), f32(sizeArr[1].number())};
  }

  return true;

  #undef FAIL_HEADER
}

bool SpeakCommand::load(const struct StoryScene &scene, const json::Object &data) {
  #define FAIL_HEADER "Could not parse story scene speak command"

  const auto *actorVal = data.get("actor"_sv);
  FAIL_IF(actorVal == nullptr, "Could not find actor for speak command.");
  FAIL_IF(!actorVal->isString(), "Expected string for actor.");
  actor = 0;
  for(; actor < scene.actors.size(); ++actor) {
    if(scene.actors[actor].id.view() == actorVal->string()) {
      break;
    }
  }
  FAIL_IF(actor == scene.actors.size(),
    "Could not find actor with id {}.", actorVal->string());

  const auto *textVal = data.get("text"_sv);
  FAIL_IF(textVal == nullptr, "Could not find text for speak command.");
  FAIL_IF(!textVal->isString(), "Expected string for text.");
  text = textVal->string();

  const auto *portraitVal = data.get("portrait"_sv);
  if(portraitVal != nullptr) {
    FAIL_IF(!portraitVal->isArray(), "Expected array for portrait.");
    auto portraitArr = portraitVal->array();
    FAIL_IF(portraitArr.size() != 2, "Expected array of size 2 for portrait.");
    auto portraitX = s32(portraitArr[0].number());
    auto portraitY = s32(portraitArr[1].number());
    const auto &actorData = scene.actors[actor];
    portrait = portraitY * actorData.sheetSize.x + portraitX;
  }

  return true;

  #undef FAIL_HEADER
}

bool WaitCommand::load(const json::Object &data) {
  #define FAIL_HEADER "Could not parse story scene wait command"

  const auto *time = data.get("time"_sv);
  FAIL_IF(time == nullptr, "Could not find time for wait command.");
  FAIL_IF(!time->isNumber(), "Expected number for time.");
  duration = f32(time->number());

  return true;

  #undef FAIL_HEADER
}

bool BackgroundCommand::load(const StoryScene &scene, const json::Object &data) {
  #define FAIL_HEADER "Could not parse story scene background command"

  const auto *imageVal = data.get("image"_sv);
  if(imageVal != nullptr) {
    FAIL_IF(imageVal == nullptr, "Could not find image for background command.");
    FAIL_IF(!imageVal->isString(), "Expected string for image.");
    const auto &imageName = imageVal->string();
    if(imageName.empty()) {
      removeBackground = true;
    } else {
      for(image = 0; image < scene.bgImages.size(); ++image) {
        if(scene.bgImages[image].view() == imageName) {
          break;
        }
      }
    }
    FAIL_IF(image == scene.bgImages.size(),
      "Could not find background with id {}.", imageName);
  }

  const auto *musicVal = data.get("music"_sv);
  if(musicVal != nullptr) {
    FAIL_IF(musicVal == nullptr, "Could not find music for background command.");
    FAIL_IF(!musicVal->isString(), "Expected string for music.");
    const auto &musicName = musicVal->string();
    if(musicName.empty()) {
      stopMusic = true;
    } else {
      for(music = 0; music < scene.musicTracks.size(); ++music) {
        if(scene.musicTracks[music].view() == musicName) {
          break;
        }
      }
    }
    FAIL_IF(music == scene.musicTracks.size(),
      "Could not find music with id {}.", musicName);
  }

  return true;

  #undef FAIL_HEADER
}

} // namespace sigmoid
