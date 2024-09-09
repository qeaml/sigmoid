#include "StoryScene.hpp"
#include "imgui/imgui.hpp"
#include "states.hpp"
#include <nwge/common/cast.hpp>
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class SceneEditorSubState final: public SubState {
public:
  SceneEditorSubState(EditorInfo &info, const StringView &sceneName, SceneType defaultType)
    : mInfo(info),
      mSceneName(sceneName),
      mScene(sceneName)
  {
    ScratchArray<char> filename = ScratchString::formatted("{}.SCN", mSceneName);
    toUpper(filename.view());
    // make sure to trim off NUL byte
    mFileName = StringView(filename.view().sub(0, filename.size() - 1));
    mScene.type = defaultType;
    nqLoadSceneInfo();
    mInfo.assets.discover();
    mInfo.scenes.discover();
  }

  bool on(Event &evt) override {
    switch(evt.type) {
    case Event::PostLoad:
      copySceneInfo();
      break;
    default:
      break;
    }
    return true;
  }

  bool tick([[maybe_unused]] f32 delta) override {
    sceneWindow();
    switch(mScene.type) {
    case SceneStory:
      actorWindow();
      commandWindow();
      break;
    default:
      break;
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
  }

private:
  EditorInfo &mInfo;
  String<> mSceneName;
  String<> mFileName;
  Scene mScene;

  static constexpr usize cBufSize = 40;
  std::array<char, cBufSize> mTitleBuf{};
  std::array<char, cBufSize> mBackgroundBuf{};
  std::array<char, cBufSize> mMusicBuf{};
  std::array<char, cBufSize> mNextBuf{};

  template<usize Size>
  static inline void safeCopyString(const nwge::StringView &src, std::array<char, Size> &dst) {
    usize size = SDL_min(src.size(), Size);
    std::copy_n(src.begin(), size, dst.data());
  }

  void copySceneInfo() {
    safeCopyString(mScene.title, mTitleBuf);
    if(mScene.background.empty()) {
      mShowBackground = false;
    } else if(mScene.background[0] != mBackgroundBuf[0]) {
      mShowBackground = true;
      mInfo.store.nqLoad(mScene.background, mBackground);
    }
    safeCopyString(mScene.background, mBackgroundBuf);
    safeCopyString(mScene.music, mMusicBuf);
    safeCopyString(mScene.next, mNextBuf);
    switch(mScene.type) {
    case SceneStory:
      copyStoryInfo();
      break;
    default:
      break;
    }
  }

  void copyStoryCommands() {
    if(mScene.story->commands.empty()) {
      mCommands.clear();
      return;
    }
    mCommands = {mScene.story->commands.size()};
    for(const auto &src: mScene.story->commands) {
      CommandInfo info{src.code};
      switch(info.code) {
      case CommandSprite:
        safeCopyString(src.sprite->id, info.idBuf);
        info.shown = !src.sprite->hide;
        if(src.sprite->pos.x != -1 && src.sprite->pos.y != -1) {
          info.move = true;
          info.posX = src.sprite->pos.x;
          info.posY = src.sprite->pos.y;
        }
        if(src.sprite->size.x != -1 && src.sprite->size.y != -1) {
          info.scale = true;
          info.sizeX = src.sprite->size.x;
          info.sizeY = src.sprite->size.y;
        }
        safeCopyString(src.sprite->actor, info.actorBuf);
        info.portraitX = src.sprite->portrait;
        break;
      case CommandSpeak:
        safeCopyString(src.speak->actor, info.actorBuf);
        info.portraitX = src.speak->portrait;
        safeCopyString(src.speak->text, info.textBuf);
        break;
      case CommandWait:
        info.waitTime = src.wait->duration;
        break;
      case CommandBackground:
        safeCopyString(src.background->background, info.backgroundBuf);
        safeCopyString(src.background->music, info.musicBuf);
        break;
      default:
        break;
      }
      mCommands.push(info);
    }
  }

  void copyStoryInfo() {
    if(!mScene.story.present()) {
      return;
    }
    mActors.clear();
    for(const auto & src: mScene.story->actors) {
      mActors.push({});
      auto &actorInfo = mActors[mActors.size() - 1];
       actorInfo.id = src.id;
      safeCopyString(src.name, actorInfo.nameBuf);
      safeCopyString(src.sheet, actorInfo.sheetBuf);
      actorInfo.sheetWidth = src.sheetSize.x;
      actorInfo.sheetHeight = src.sheetSize.y;
    }
    copyStoryCommands();
  }

  void nqLoadSceneInfo() {
    mInfo.store.nqLoad(mFileName, mScene);
  }

  void nqSaveSceneInfo() {
    mScene.title = mTitleBuf.data();
    mScene.background = mBackgroundBuf.data();
    mScene.music = mMusicBuf.data();
    mScene.next = mNextBuf.data();
    switch(mScene.type) {
    case SceneStory:
      setUpStoryInfo();
      break;
    default:
      break;
    }
    mInfo.store.nqSave(mFileName, mScene);
  }

  void setUpStoryInfo() {
    if(!mScene.story.present()) {
      mScene.story.emplace();
    }
    setUpStoryActors();
    setUpStoryCommands();
  }

  void setUpStoryActors() {
    mScene.story->actors.clear();
    if(mActors.size() == 0) {
      return;
    }
    mScene.story->actors = {mActors.size()};
    for(const auto & src : mActors) {
      Actor actor;
      actor.id = src.id;
      actor.name = src.nameBuf.data();
      actor.sheet = src.sheetBuf.data();
      actor.sheetSize = {
        src.sheetWidth,
        src.sheetHeight
      };
      mScene.story->actors.push(actor);
    }
  }

  void setUpStoryCommands() {
    if(mCommands.size() == 0) {
      mScene.story->commands = {};
      return;
    }
    mScene.story->commands = {mCommands.size()};
    for(usize i = 0; i < mCommands.size(); ++i) {
      auto &command = mScene.story->commands[i];
      const auto &src = mCommands[i];
      command.code = src.code;
      switch(src.code) {
      case CommandSprite:
        command.sprite.emplace();
        command.sprite->id = src.idBuf.data();
        command.sprite->hide = !src.shown;
        if(src.move) {
          command.sprite->pos.x = src.posX;
          command.sprite->pos.y = src.posY;
        }
        if(src.scale) {
          command.sprite->size.x = src.sizeX;
          command.sprite->size.y = src.sizeY;
        }
        command.sprite->actor = src.actorBuf.data();
        command.sprite->portrait = src.portraitX;
        break;
      case CommandSpeak:
        command.speak.emplace();
        command.speak->actor = src.actorBuf.data();
        command.speak->portrait = src.portraitX;
        command.speak->text = src.textBuf.data();
        break;
      case CommandWait:
        command.wait.emplace();
        command.wait->duration = src.waitTime;
        break;
      case CommandBackground:
        command.background.emplace();
        command.background->background = src.backgroundBuf.data();
        command.background->music = src.musicBuf.data();
        break;
      default:
        break;
      }
      mScene.story->commands[i] = command;
    }
  }

  render::AspectRatio m1x1{1, 1};
  render::AspectRatio m4x3{4, 3};
  bool mShowBackground = false;
  render::Texture mBackground;

  void sceneWindow() {
    if(!ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return;
    }

    ImGui::Text("Name: %s", mSceneName.begin());
    ImGui::Text("Filename: %s", mFileName.begin());
    if(mShowBackground) {
      if(ImGui::Button("Hide background")) {
        mShowBackground = false;
      }
    }
    ImGui::InputText("Title", mTitleBuf.data(), cBufSize);
    ImGui::InputText("Background", mBackgroundBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);
    ImGui::InputText("Music", mMusicBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);
    ImGui::InputText("Next", mNextBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);

    if(ImGui::Button("Save")) {
      nqSaveSceneInfo();
    }
    ImGui::SameLine();
    if(ImGui::Button("Restore")) {
      nqLoadSceneInfo();
    }
    ImGui::SameLine();
    if(ImGui::Button("Return")) {
      setEditorSubState(gameInfoEditor(mInfo));
    }

    ImGui::End();
  }

  std::array<char, cBufSize> mActorIdBuf{};
  struct ActorInfo {
    String<> id;
    std::array<char, cBufSize> nameBuf{};
    std::array<char, cBufSize> sheetBuf{};
    s32 sheetWidth = 1;
    s32 sheetHeight = 1;
  };
  Slice<ActorInfo> mActors{4};
  ssize mSelectedActor = -1;

  void actorWindow() {
    if(!ImGui::Begin("Actors", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return;
    }

    if(ImGui::BeginListBox("##actors")) {
      for(usize i = 0; i < mActors.size(); ++i) {
        ActorInfo &actor = mActors[i];
        auto idx = saturate_cast<ssize>(i);
        if(ImGui::Selectable(actor.id.begin(), idx == mSelectedActor)) {
          mSelectedActor = idx;
        }
      }
      ImGui::EndListBox();
    }

    if(mSelectedActor >= 0) {
      ActorInfo &actor = mActors[mSelectedActor];
      ImGui::InputText("Name", actor.nameBuf.data(), cBufSize);
      ImGui::InputText("Sheet", actor.sheetBuf.data(), cBufSize,
        ImGuiInputTextFlags_CharsUppercase);
      ImGui::InputInt("Sheet width", &actor.sheetWidth);
      ImGui::InputInt("Sheet height", &actor.sheetHeight);
      if(ImGui::Button("Deselect")) {
        mSelectedActor = -1;
      }
      ImGui::SameLine();
      if(ImGui::Button("Delete")) {
        Slice<ActorInfo> newActors(mActors.size() - 1);
        for(auto &actorInfo: mActors) {
          if(actor.id.view() != actorInfo.id) {
            newActors.push(std::move(actorInfo));
          }
        }
        mActors = std::move(newActors);
      }
    } else {
      ImGui::InputText("Actor ID", mActorIdBuf.data(), cBufSize);
      if(ImGui::Button("Add actor")) {
        ActorInfo actor;
        actor.id = mActorIdBuf.data();
        mActors.push({actor});
        mSelectedActor = saturate_cast<ssize>(mActors.size()) - 1;
      }
    }

    ImGui::End();
  }

  struct CommandInfo {
    CommandCode code = CommandInvalid;

    // used by sprite command
    std::array<char, cBufSize> idBuf{};
    bool shown = true;
    bool move = false;
    f32 posX = 0;
    f32 posY = 0;
    bool scale = false;
    f32 sizeX = 0;
    f32 sizeY = 0;

    // used by speak & sprite
    std::array<char, cBufSize> actorBuf{};
    s32 portraitX = 0;
    s32 portraitY = 0;

    // used by speak
    std::array<char, cBufSize> textBuf{};

    // used by wait
    f32 waitTime = 0;

    // used by background
    std::array<char, cBufSize> backgroundBuf{};
    std::array<char, cBufSize> musicBuf{};
  };
  Slice<CommandInfo> mCommands{4};
  ssize mSelectedCommand = -1;

  void commandWindow() {
    if(!ImGui::Begin("Commands", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return;
    }

    if(ImGui::BeginListBox("##commands")) {
      for(usize i = 0; i < mCommands.size(); ++i) {
        CommandInfo &command = mCommands[i];
        auto idx = saturate_cast<ssize>(i);
        if(ImGui::Selectable(cCommandCodeNames[command.code], idx == mSelectedCommand)) {
          mSelectedCommand = idx;
        }
      }
      ImGui::EndListBox();
    }

    if(mSelectedCommand < 0) {
      if(ImGui::Button("Add Sprite Command")) {
        mCommands.push({CommandSprite});
        mSelectedCommand = saturate_cast<ssize>(mCommands.size()) - 1;
      }
      if(ImGui::Button("Add Speak Command")) {
        mCommands.push({CommandSpeak});
        mSelectedCommand = saturate_cast<ssize>(mCommands.size()) - 1;
      }
      if(ImGui::Button("Add Wait Command")) {
        mCommands.push({CommandWait});
        mSelectedCommand = saturate_cast<ssize>(mCommands.size()) - 1;
      }
      if(ImGui::Button("Add Background Command")) {
        mCommands.push({CommandBackground});
        mSelectedCommand = saturate_cast<ssize>(mCommands.size()) - 1;
      }
    } else {
      if(ImGui::Button("Deselect")) {
        mSelectedCommand = -1;
      }
      ImGui::SameLine();
      if(ImGui::Button("Delete")) {
        Slice<CommandInfo> newCommands(mCommands.size() - 1);
        for(usize i = 0; i < mCommands.size(); ++i) {
          auto idx = saturate_cast<ssize>(i);
          if(idx != mSelectedCommand) {
            newCommands.push(mCommands[i]);
          }
        }
        mCommands = std::move(newCommands);
        mSelectedCommand = -1;
      }
    }

    if(mSelectedCommand >= 0) {
      auto &info = mCommands[mSelectedCommand];
      switch(info.code) {
      case CommandSprite:
        spriteCommandOptions(info);
        break;
      case CommandSpeak:
        speakCommandOptions(info);
        break;
      case CommandWait:
        waitCommandOptions(info);
        break;
      case CommandBackground:
        backgroundCommandOptions(info);
        break;
      default:
        break;
      }
    }

    ImGui::End();
  }

  static void spriteCommandOptions(CommandInfo &info) {
    ImGui::InputText("ID", info.idBuf.data(), cBufSize);
    ImGui::Checkbox("Shown", &info.shown);

    ImGui::Checkbox("Change Position", &info.move);
    ImGui::InputFloat("Position X", &info.posX);
    ImGui::InputFloat("Position Y", &info.posY);

    ImGui::Checkbox("Change Size", &info.scale);
    ImGui::InputFloat("Size X", &info.sizeX);
    ImGui::InputFloat("Size Y", &info.sizeY);

    ImGui::InputText("Actor", info.actorBuf.data(), cBufSize);
    ImGui::InputInt("Portrait X", &info.portraitX);
    ImGui::InputInt("Portrait Y", &info.portraitY);
  }

  static void speakCommandOptions(CommandInfo &info) {
    ImGui::InputText("Actor", info.actorBuf.data(), cBufSize);
    ImGui::InputInt("Portrait X", &info.portraitX);
    ImGui::InputInt("Portrait Y", &info.portraitY);
    ImGui::InputText("Text", info.textBuf.data(), cBufSize);
  }

  static void waitCommandOptions(CommandInfo &info) {
    ImGui::InputFloat("Wait Time", &info.waitTime);
  }

  static void backgroundCommandOptions(CommandInfo &info) {
    ImGui::InputText("Background", info.backgroundBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);
    ImGui::InputText("Music", info.musicBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);
  }
};

SubState *sceneEditor(EditorInfo &info, const StringView &sceneName, SceneType defaultType) {
  return new SceneEditorSubState(info, sceneName, defaultType);
}

} // namespace sigmoid
