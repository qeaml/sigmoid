#include "SceneManager.hpp"
#include "states.hpp"
#include "AssetManager.hpp"
#include "imgui/imgui.hpp"
#include <nwge/data/file.hpp>
#include <nwge/data/store.hpp>
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class EditorStorySceneState final: public State {
public:
  EditorStorySceneState(const StringView &gameName, const StringView &sceneName)
    : mAssets(gameName), mScenes(gameName), mGameName(gameName), mSceneName(sceneName), mStore(gameName)
  {
    ScratchArray<char> filename = ScratchString::formatted("{}.SCN", mSceneName);
    toUpper(filename.view());
    // make sure to trim off NUL byte
    mFileName = StringView(filename.view().sub(0, filename.size() - 1));
  }

  bool preload() override {
    nqLoadSceneInfo();
    mAssets.discover();
    mScenes.discover();
    return true;
  }

  bool init() override {
    copySceneInfo();
    return true;
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
    mAssets.window();
    mScenes.window();
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
  render::AspectRatio m1x1{1, 1};
  render::AspectRatio m4x3{4, 3};

  AssetManager mAssets;
  SceneManager mScenes;

  String<> mGameName;
  String<> mSceneName;
  String<> mFileName;
  data::Store mStore;
  Maybe<Scene> mScene;

  bool mShowBackground = false;
  render::Texture mBackground;

  void initScene() {
    mScene.emplace(mSceneName);
    mScene->type = SceneStory;
    mScene->story.emplace();
  }

  void nqLoadSceneInfo() {
    initScene();
    mStore.nqLoad(mFileName, *mScene);
  }

  void copySceneInfo() {
    if(mScene.present()) {
      safeCopyString(mScene->title, mTitleBuf);
      if(mScene->background[0] == 0) {
        mShowBackground = false;
      } else if(mScene->background[0] != mBackgroundBuf[0]) {
        mShowBackground = true;
        mStore.nqLoad(mScene->background, mBackground);
      }
      safeCopyString(mScene->background, mBackgroundBuf);
      safeCopyString(mScene->music, mMusicBuf);
      safeCopyString(mScene->next, mNextBuf);
      mScene.clear();
    }
  }

  void createSceneInfo() {
    initScene();
    mScene->title = mTitleBuf.data();
    mScene->background = mBackgroundBuf.data();
    mScene->music = mMusicBuf.data();
    mScene->next = mNextBuf.data();
  }

  void nqSaveSceneInfo() {
    createSceneInfo();
    mStore.nqSave(mFileName, *mScene);
  }

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
    if(ImGui::Button("Show")) {
      mShowBackground = true;
      mStore.nqLoad(mBackgroundBuf.begin(), mBackground);
    }
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
      swapStatePtr(editorGameMenu(mGameName));
    }

    ImGui::End();
  }

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
};

State *editorStoryScene(const StringView &gameName, const StringView &sceneName) {
  return new EditorStorySceneState(gameName, sceneName);
}

} // namespace sigmoid
