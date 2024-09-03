#include "StoryScene.hpp"
#include "states.hpp"
#include <nwge/bind.hpp>
#include <nwge/common/maybe.hpp>
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class StorySceneSubState final: public SubState {
public:
  /*
  Here's your cursory reminder that we can do whatever we want in the SubState
  constructor because we know that it's only invoked from within
  SceneState::init() :3
  */
  StorySceneSubState(SceneStateData &data)
    : mData(data),
      mActors(mStory.actors.size())
  {
    initActors();

    const auto &background = mData.scene.background();
    if(!background.empty()) {
      showBackground(background);
    }
  }

  bool tick(f32 delta) override {
    if(mCommandOff >= mStory.commands.size()) {
      dialog::info("StorySceneSubState"_sv, "Finished."_sv);
      return false;
    }
    if(!mWaitForInput) {
      nextCommand();
    }
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
    if(mShowBackground) {
      m4x3.rect({
        {0, 0, 0.9},
        {1, 1},
        mBackground.id
      }).draw();
    }

    render::color();
  }

private:
  SceneStateData &mData;
  const StoryScene &mStory = mData.scene.story();

  render::AspectRatio m1x1{1, 1};
  render::AspectRatio m4x3{4, 3};

  bool mShowBackground = false;
  render::Texture mBackground;

  void removeBackground() {
    mShowBackground = false;
  }

  void showBackground(const StringView &name) {
    mShowBackground = true;
    mData.game.bundle().nqTexture(name, mBackground);
  }

  void backgroundCmd(const BackgroundCommand &cmd) {
    if(cmd.removeBackground) {
      removeBackground();
    } else if(cmd.image >= 0) {
      showBackground(mStory.bgImages[cmd.image]);
    }
    if(cmd.stopMusic) {
      console::warn("Music not yet implemented. (stopMusic)");
    } else if(cmd.music >= 0) {
      console::warn("Music not yet implemented. (setMusic)");
    }
  }

  struct ActorInfo {
    String<> name;
    render::Texture sheet;
    Array<render::TexCoord> sprites;
  };
  Array<ActorInfo> mActors;

  void initActors() {
    for(usize i = 0; i < mStory.actors.size(); ++i) {
      const auto &actor = mStory.actors[i];
      ActorInfo &info = mActors[i];

      info.name = actor.name;

      mData.game.bundle().nqTexture(actor.sheet, info.sheet);
      usize spriteCount = usize(actor.sheetSize.x) *  usize(actor.sheetSize.y);
      info.sprites = {spriteCount};
      for(usize j = 0; j < spriteCount; ++j) {
        info.sprites[j] = {
          {
            f32(j % actor.sheetSize.x) / f32(actor.sheetSize.x),
            f32(s32(j / actor.sheetSize.x)) / f32(actor.sheetSize.y)
          },
          {
            1.0f / f32(actor.sheetSize.x),
            1.0f / f32(actor.sheetSize.y)
          }
        };
      }
    }
  }

  void speakCmd(const SpeakCommand &cmd) {
    mWaitForInput = true;
    console::print("{}: {}",
      mActors[cmd.actor].name, cmd.text);
  }

  usize mCommandOff = 0;
  bool mWaitForInput = false;

  void nextCommand() {
    mWaitForInput = false;
    const auto &command = mStory.commands[mCommandOff++];
    switch(command.code) {
    case CommandSprite:
      console::warn("Sprite command not implemented.");
      break;
    case CommandSpeak:
      speakCmd(*command.speak);
      break;
    case CommandWait:
      console::warn("Wait command not implemented.");
      break;
    case CommandBackground:
      backgroundCmd(*command.background);
      break;
    default:
      console::error("Unknown command.");
      break;
    }
  }

  KeyBind mBindNext{"story.next"_sv, Key::Space, [this]{
    if(mWaitForInput) {
      nextCommand();
    }
  }};
};

SubState *storyScene(SceneStateData &data) {
  return new StorySceneSubState(data);
}

} // namespace sigmoid
