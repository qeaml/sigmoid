#include "imgui/imgui.hpp"
#include "states.hpp"
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

  bool tick(f32 delta) override {
    sceneWindow();
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
  }

  void nqLoadSceneInfo() {
    mInfo.store.nqLoad(mFileName, mScene);
  }

  void nqSaveSceneInfo() {
    mScene.title = mTitleBuf.data();
    mScene.background = mBackgroundBuf.data();
    mScene.music = mMusicBuf.data();
    mScene.next = mNextBuf.data();
    mInfo.store.nqSave(mFileName, mScene);
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
};

SubState *sceneEditor(EditorInfo &info, const StringView &sceneName, SceneType defaultType) {
  return new SceneEditorSubState(info, sceneName, defaultType);
}

} // namespace sigmoid
