#include "StoryScene.hpp"
#include <nwge/dialog.hpp>

using namespace nwge;

namespace sigmoid {

bool StoryScene::load(const json::Object &data) {
  const auto *actorsVal = data.get("actors"_sv);
  if(actorsVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse story scene:\n"
      "Could not find actors.");
    return false;
  }
  if(!actorsVal->isObject()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene:\n"
      "Expected object for actors.");
    return false;
  }
  if(!loadActors(actorsVal->object())) {
    return false;
  }

  const auto *commandsVal = data.get("commands"_sv);
  if(commandsVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse story scene:\n"
      "Could not find commands.");
    return false;
  }
  if(!commandsVal->isArray()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene:\n"
      "Expected array for commands.");
    return false;
  }
  if(!loadCommands(commandsVal->array())) {
    return false;
  }

  return true;
}

bool StoryScene::loadActors(const json::Object &data) {
  actors = {data.size()};
  usize idx = 0;
  for(const auto &[key, val]: data.pairs()) {
    if(!val.isObject()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene actors:\n"
        "Expected object for actor {}.",
        key);
      return false;
    }
    const auto &obj = val.object();
    auto &actor = actors[idx++];
    if(!actor.load(key, obj)) {
      return false;
    }
  }
  return true;
}

bool Actor::load(const StringView &actorID, const json::Object &data) {
  id = actorID;
  const auto *nameVal = data.get("name"_sv);
  if(nameVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Could not find name for actor {}.",
      actorID);
    return false;
  }
  if(!nameVal->isString()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Expected string for actor {} name.",
      actorID);
    return false;
  }
  name = nameVal->string();

  const auto *sheetNameVal = data.get("sheet"_sv);
  if(sheetNameVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Could not find sheet for actor {}.",
      actorID);
    return false;
  }
  if(!sheetNameVal->isString()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Expected string for actor {} sheet.",
      actorID);
    return false;
  }
  sheetName = sheetNameVal->string();

  const auto *spritesVal = data.get("sprites"_sv);
  if(spritesVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Could not find sprites for actor {}.",
      actorID);
    return false;
  }
  if(!spritesVal->isArray()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Expected array for actor {} sprites.",
      actorID);
    return false;
  }
  auto spriteArr = spritesVal->array();
  if(spriteArr.size() != 2) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Sheet size must consist of 2 numbers for actor {}.",
      actorID);
    return false;
  }
  const auto &xVal = spriteArr[0];
  const auto &yVal = spriteArr[1];
  if(!xVal.isNumber() || !yVal.isNumber()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene actors:\n"
      "Sheet size must consist of 2 numbers for actor {}.",
      actorID);
    return false;
  }
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
}

bool StoryScene::loadSprites(const ArrayView<json::Value> &commandData) {
  Slice<const json::Object*> spriteCommands{actors.size()};
  for(const auto &val: commandData) {
    if(!val.isObject()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprites:\n"
        "Expected object for command.");
      return false;
    }
    const auto &obj = val.object();
    const auto *spriteVal = obj.get("sprite"_sv);
    if(spriteVal == nullptr) {
      continue;
    }
    if(!spriteVal->isObject()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprites:\n"
        "Expected object for sprite command.");
      return false;
    }
    const auto &spriteObj = spriteVal->object();
    spriteCommands.push(&spriteObj);
  }

  Slice<Sprite> spriteData{spriteCommands.size()};
  for(const auto *command: spriteCommands) {
    const auto *idVal = command->get("id"_sv);
    if(idVal == nullptr) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprites:\n"
        "Could not find id for sprite command.");
      return false;
    }
    if(!idVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprites:\n"
        "Expected string for sprite command id.");
      return false;
    }
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
}

bool StoryScene::loadCommands(const ArrayView<json::Value> &data) {
  commands = {data.size()};
  for(usize i = 0; i < data.size(); ++i) {
    const auto &command = data[i];
    if(!command.isObject()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene commands:\n"
        "Expected object for command {}.",
        i);
      return false;
    }
    auto &cmd = commands[i];
    const auto &obj = command.object();

    const auto *data = obj.get("sprite"_sv);
    if(data != nullptr) {
      if(!data->isObject()) {
        dialog::error("Failure"_sv,
          "Could not parse story scene commands:\n"
          "Expected object for sprite command {}.",
          i);
        return false;
      }
      cmd.code = CommandSprite;
      cmd.sprite.emplace();
      if(!cmd.sprite->load(*this, data->object())) {
        return false;
      }
      continue;
    }

    data = obj.get("speak"_sv);
    if(data != nullptr) {
      if(!data->isObject()) {
        dialog::error("Failure"_sv,
          "Could not parse story scene commands:\n"
          "Expected object for speak command {}.",
          i);
        return false;
      }
      cmd.code = CommandSpeak;
      cmd.speak.emplace();
      if(!cmd.speak->load(data->object())) {
        return false;
      }
      continue;
    }

    data = obj.get("wait"_sv);
    if(data != nullptr) {
      if(!data->isObject()) {
        dialog::error("Failure"_sv,
          "Could not parse story scene commands:\n"
          "Expected object for wait command {}.",
          i);
        return false;
      }
      cmd.code = CommandWait;
      cmd.wait.emplace();
      if(!cmd.wait->load(data->object())) {
        return false;
      }
      continue;
    }

    data = obj.get("background"_sv);
    if(data != nullptr) {
      if(!data->isObject()) {
        dialog::error("Failure"_sv,
          "Could not parse story scene commands:\n"
          "Expected object for background command {}.",
          i);
        return false;
      }
      cmd.code = CommandBackground;
      cmd.background.emplace();
      if(!cmd.background->load(data->object())) {
        return false;
      }
      continue;
    }

    dialog::error("Failure"_sv,
      "Could not parse story scene commands:\n"
      "Invalid command {}.",
      i);
    return false;
  }

  return true;
}

bool SpriteCommand::load(const StoryScene &scene, const json::Object &data) {
  const auto *idVal = data.get("id"_sv);
  if(idVal == nullptr) {
    dialog::error("Failure"_sv,
      "Could not parse story scene sprite command:\n"
      "Could not find id.");
    return false;
  }
  if(!idVal->isString()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene sprite command:\n"
      "Expected string for id.");
    return false;
  }
  const auto &spriteID = idVal->string();
  usize sprite = 0;
  for(; sprite < scene.sprites.size(); ++sprite) {
    if(scene.sprites[sprite].id.view() == spriteID) {
      break;
    }
  }
  if(sprite == scene.sprites.size()) {
    dialog::error("Failure"_sv,
      "Could not parse story scene sprite command:\n"
      "Could not find sprite with id {}.",
      spriteID);
    return false;
  }

  const auto *hideVal = data.get("hide"_sv);
  if(hideVal != nullptr) {
    if(!hideVal->isBoolean()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected bool for hide.");
      return false;
    }
    hide = hideVal->boolean();
  }

  const auto *actorVal = data.get("actor"_sv);
  if(actorVal != nullptr) {
    if(!actorVal->isString()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected string for actor.");
      return false;
    }
    actor = 0;
    for(; actor < scene.actors.size(); ++actor) {
      if(scene.actors[actor].id.view() == actorVal->string()) {
        break;
      }
    }
    if(actor == scene.actors.size()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Could not find actor with id {}.",
        actorVal->string());
      return false;
    }
  }

  const auto *portraitVal = data.get("portrait"_sv);
  if(portraitVal != nullptr) {
    if(!portraitVal->isArray()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected array for portrait.");
      return false;
    }
    auto portraitArr = portraitVal->array();
    if(portraitArr.size() != 2) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected array of size 2 for portrait.");
      return false;
    }
    auto portraitX = s32(portraitArr[0].number());
    auto portraitY = s32(portraitArr[1].number());
    const auto &actorData = scene.actors[actor];
    portrait = portraitY * actorData.sheetSize.x + portraitX;
  }

  const auto *posVal = data.get("pos"_sv);
  if(posVal != nullptr) {
    if(!posVal->isArray()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected array for pos.");
      return false;
    }
    auto posArr = posVal->array();
    if(posArr.size() != 2) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected array of size 2 for pos.");
      return false;
    }
    pos = {s32(posArr[0].number()), s32(posArr[1].number())};
  }

  const auto *sizeVal = data.get("size"_sv);
  if(sizeVal != nullptr) {
    if(!sizeVal->isArray()) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected array for size.");
      return false;
    }
    auto sizeArr = sizeVal->array();
    if(sizeArr.size() != 2) {
      dialog::error("Failure"_sv,
        "Could not parse story scene sprite command:\n"
        "Expected array of size 2 for size.");
      return false;
    }
    size = {s32(sizeArr[0].number()), s32(sizeArr[1].number())};
  }

  return true;
}

// TODO:
//  * SpeakCommand::load
//  * WaitCommand::load
//  * BackgroundCommand::load

} // namespace sigmoid
