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
    mScene.story->actors = {mActors.size()};
    for(usize i = 0; i < mActors.size(); ++i) {
      auto &actor = mScene.story->actors[i];
      const auto &src = mActors[i];
      actor.id = src.id;
      actor.name = src.nameBuf.data();
      actor.sheet = src.sheetBuf.data();
      actor.sheetSize = {
        src.sheetWidth,
        src.sheetHeight
      };
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
};

SubState *sceneEditor(EditorInfo &info, const StringView &sceneName, SceneType defaultType) {
  return new SceneEditorSubState(info, sceneName, defaultType);
}

} // namespace sigmoid
