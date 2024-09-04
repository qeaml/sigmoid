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
    if(!mCurrentText.empty()) {
      if(mTextChars < mCurrentText.size()) {
        mTextTimer += delta;
        if(mTextTimer >= cTextCharTime) {
          mTextChars++;
          mTextTimer = 0.0f;
        }
        return true;
      }
    }
    if(!mWaitForInput) {
      nextCommand();
      return true;
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

    if(!mCurrentText.empty()) {
      renderTextBox();
    }
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

  usize mCurrentActor = 0; // only matters if !mCurrentText.empty()
  usize mActorPortrait = 0;
  StringView mCurrentText;
  usize mTextChars = 0;
  f32 mTextTimer = 0.0f;

  void speakCmd(const SpeakCommand &cmd) {
    if(cmd.actor >= 0) {
      mCurrentActor = cmd.actor;
    }
    if(cmd.portrait >= 0) {
      mActorPortrait = cmd.portrait;
    }
    mCurrentText = cmd.text;
    mTextChars = 0;
    mTextTimer = 0.0f;
    mWaitForInput = true;
  }

  static constexpr f32 cTextCharTime = 0.05f;

  static constexpr glm::vec4 cBgColor{0, 0, 0, 0.5f};
  static constexpr f32 cTextMargin = 0.01f;
  static constexpr f32 cBaseY = 0.7f;
  static constexpr f32 cActorNameTextHeight = 0.05f;
  static constexpr glm::vec3 cActorNameBgPos{0.05f, cBaseY, 0.51f};
  static constexpr f32 cActorNameBgHeight = cActorNameTextHeight + 2*cTextMargin;
  static constexpr glm::vec3 cActorNameUnderlineStart{
    cActorNameBgPos.x, cActorNameBgPos.y + cActorNameBgHeight, 0.50f
  };
  static constexpr glm::vec3 cActorNameTextPos{
    cActorNameBgPos.x + cTextMargin, cActorNameBgPos.y + cTextMargin, 0.50f
  };
  static constexpr glm::vec3 cTextBgPos{
    0.05f, cActorNameBgPos.y + cActorNameBgHeight, 0.51f
  };
  static constexpr f32 cTextHeight = 0.05f;
  static constexpr glm::vec2 cTextBgSize{
    1.0f,
    4*cTextMargin + 3*cTextHeight
  };
  static constexpr glm::vec3 cTextPos{
    cTextBgPos.x+cTextMargin, cTextBgPos.y+cTextMargin, 0.5f
  };
  static constexpr f32 cActorPortraitEndX = 0.95f;

  void renderTextBox() const {
    const auto &actor = mActors[mCurrentActor];
    const auto &name = actor.name;
    auto measure = mData.font.measure(name, cActorNameTextHeight);
    glm::vec2 nameBgSize{
      measure.x + 2*cTextMargin, cActorNameBgHeight
    };
    render::color(cBgColor);
    render::rect(
      m4x3.pos(cActorNameBgPos),
      nameBgSize
    );
    renderRect(cActorNameBgPos, nameBgSize);
    glm::vec3 textBgPos = m4x3.pos(cTextBgPos);
    glm::vec2 textBgSize = m1x1.size(cTextBgSize);
    render::rect(textBgPos, textBgSize);
    render::color();
    render::line(
      m4x3.pos(cActorNameUnderlineStart),
      m4x3.pos(cActorNameUnderlineStart) + glm::vec3{nameBgSize.x, 0, 0},
      1
    );
    renderText(name, cActorNameTextPos, cActorNameTextHeight);
    renderText(mCurrentText.sub(0, mTextChars), cTextPos, cTextHeight);

    glm::vec3 actorPortraitPos = textBgPos + glm::vec3(textBgSize.x, 0, 0);
    glm::vec2 actorPortraitSize = m1x1.size({cTextBgSize.y, cTextBgSize.y});
    render::rect(
      actorPortraitPos,
      actorPortraitSize,
      actor.sheet,
      actor.sprites[mActorPortrait]
    );
  }

  constexpr inline void renderRect(glm::vec3 pos, glm::vec2 extents) const {
    render::rect(m4x3.pos(pos), m1x1.size(extents));
  }

  constexpr inline void renderText(const StringView &text, glm::vec3 pos, f32 size) const {
    mData.font.draw(text, m4x3.pos(pos), size);
  }

  usize mCommandOff = 0;
  bool mWaitForInput = false;

  void nextCommand() {
    if(mCommandOff >= mStory.commands.size()) {
      popSubState();
      return;
    }

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
    if(mWaitForInput && mTextChars >= mCurrentText.size()) {
      mCurrentText = {};
      nextCommand();
    }
  }};
};

SubState *storyScene(SceneStateData &data) {
  return new StorySceneSubState(data);
}

} // namespace sigmoid
