#include "Scene.hpp"
#include "SceneManager.hpp"
#include "states.hpp"
#include "AssetManager.hpp"
#include "imgui/imgui.hpp"
#include <nwge/common/cast.hpp>
#include <nwge/data/file.hpp>
#include <nwge/data/store.hpp>
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class EditorGameMenuState final: public State {
public:
  EditorGameMenuState(const nwge::StringView &gameName)
    : mStore(gameName), mName(gameName), mScenes(gameName), mAssets(gameName)
  {}

  bool preload() override {
    nqLoadGameInfo();
    mScenes.discover();
    mAssets.discover();
    return true;
  }

  bool init() override {
    copyGameInfo();
    return true;
  }

  bool on(Event &evt) override {
    switch(evt.type) {
    case Event::PostLoad:
      copyGameInfo();
      break;
    default:
      break;
    }
    return true;
  }

  bool tick([[maybe_unused]] f32 delta) override {
    gameInfoWindow();
    mScenes.window();
    mAssets.window();
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

  data::Store mStore;
  String<> mName;
  Maybe<Game> mGameInfo;

  void copyGameInfo() {
    if(mGameInfo.present()) {
      safeCopyString(mGameInfo->title, mTitleBuf);
      safeCopyString(mGameInfo->author, mAuthorBuf);
      safeCopyString(mGameInfo->description, mDescriptionBuf);
      safeCopyString(mGameInfo->version, mVersionBuf);
      safeCopyString(mGameInfo->logo, mLogoBuf);
      if(mGameInfo->menuBackground.empty()) {
        mShowBackground = false;
      } else if(mGameInfo->menuBackground[0] != mMenuBackgroundBuf[0]) {
        mShowBackground = true;
        mStore.nqLoad(mGameInfo->menuBackground, mBackground);
      }
      safeCopyString(mGameInfo->menuBackground, mMenuBackgroundBuf);
      safeCopyString(mGameInfo->startScene, mStartSceneBuf);
      mGameInfo.clear();
    }
  }

  SceneManager mScenes;
  AssetManager mAssets;

  bool mShowBackground = false;
  render::Texture mBackground;

  template<usize Size>
  static inline void safeCopyString(const nwge::StringView &src, std::array<char, Size> &dst) {
    usize size = SDL_min(src.size(), Size);
    std::copy_n(src.begin(), size, dst.data());
  }

  void nqLoadGameInfo() {
    mStore.nqLoad("GAME.INFO", [this](auto &file){
      mGameInfo = Game(""_sv);
      return mGameInfo->load(file);
    });
  }

  void nqSaveGameInfo() {
    mStore.nqSave("GAME.INFO"_sv, [this](auto &file) {
      mGameInfo = Game(""_sv);
      mGameInfo->title = mTitleBuf.data();
      mGameInfo->author = mAuthorBuf.data();
      mGameInfo->description = mDescriptionBuf.data();
      mGameInfo->version = mVersionBuf.data();
      mGameInfo->logo = mLogoBuf.data();
      mGameInfo->menuBackground = mMenuBackgroundBuf.data();
      mGameInfo->startScene = mStartSceneBuf.data();
      return mGameInfo->save(file);
    });
  }

  static constexpr usize cBufSize = 40;
  std::array<char, cBufSize> mTitleBuf{};
  std::array<char, cBufSize> mAuthorBuf{};
  static constexpr usize cDescriptionBufSize = 4*cBufSize;
  std::array<char, cDescriptionBufSize> mDescriptionBuf{};
  std::array<char, cBufSize> mVersionBuf{};
  std::array<char, cBufSize> mLogoBuf{};
  std::array<char, cBufSize> mMenuBackgroundBuf{};
  std::array<char, cBufSize> mStartSceneBuf{};

  void gameInfoWindow() {
    if(!ImGui::Begin("Game Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return;
    }

    ImGui::InputText("Title", mTitleBuf.data(), cBufSize);
    ImGui::InputText("Author", mAuthorBuf.data(), cBufSize);
    ImGui::InputTextMultiline("Description", mDescriptionBuf.data(), cDescriptionBufSize);
    ImGui::InputText("Version", mVersionBuf.data(), cBufSize);
    ImGui::InputText("Logo",
      mLogoBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);
    ImGui::InputText("Menu Background",
      mMenuBackgroundBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);
    ImGui::InputText("Start Scene",
      mStartSceneBuf.data(), cBufSize,
      ImGuiInputTextFlags_CharsUppercase);

    if(ImGui::Button("Save")) {
      nqSaveGameInfo();
    }
    ImGui::SameLine();
    if(ImGui::Button("Reload")) {
      nqLoadGameInfo();
    }
    ImGui::SetItemTooltip("Reloads the game info, discarding changes");

    ImGui::End();
  }
};

State *editorGameMenu(const StringView &gameName) {
  return new EditorGameMenuState(gameName);
}

} // namespace sigmoid
