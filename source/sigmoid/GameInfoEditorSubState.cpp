#include "imgui/imgui.hpp"
#include "states.hpp"
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class GameInfoEditorSubState final: public SubState {
public:
  GameInfoEditorSubState(EditorInfo &info)
    : mInfo(info)
  {
    nqLoadGameInfo();
    mInfo.assets.discover();
    mInfo.scenes.discover();
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

  static constexpr usize cBufSize = 40;
  std::array<char, cBufSize> mTitleBuf{};
  std::array<char, cBufSize> mAuthorBuf{};
  static constexpr usize cDescriptionBufSize = 4*cBufSize;
  std::array<char, cDescriptionBufSize> mDescriptionBuf{};
  std::array<char, cBufSize> mVersionBuf{};
  std::array<char, cBufSize> mLogoBuf{};
  std::array<char, cBufSize> mMenuBackgroundBuf{};
  std::array<char, cBufSize> mStartSceneBuf{};

  template<usize Size>
  static inline void safeCopyString(const nwge::StringView &src, std::array<char, Size> &dst) {
    usize size = SDL_min(src.size(), Size);
    std::copy_n(src.begin(), size, dst.data());
  }

  void copyGameInfo() {
    safeCopyString(mInfo.game.title, mTitleBuf);
    safeCopyString(mInfo.game.author, mAuthorBuf);
    safeCopyString(mInfo.game.description, mDescriptionBuf);
    safeCopyString(mInfo.game.version, mVersionBuf);
    safeCopyString(mInfo.game.logo, mLogoBuf);
    if(mInfo.game.menuBackground.empty()) {
      mShowBackground = false;
    } else if(mInfo.game.menuBackground[0] != mMenuBackgroundBuf[0]) {
      mShowBackground = true;
      mInfo.store.nqLoad(mInfo.game.menuBackground, mBackground);
    }
    safeCopyString(mInfo.game.menuBackground, mMenuBackgroundBuf);
    safeCopyString(mInfo.game.startScene, mStartSceneBuf);
  }

  void nqLoadGameInfo() {
    mInfo.store.nqLoad("GAME.INFO", mInfo.game);
  }

  void nqSaveGameInfo() {
    mInfo.game.title = mTitleBuf.data();
    mInfo.game.author = mAuthorBuf.data();
    mInfo.game.description = mDescriptionBuf.data();
    mInfo.game.version = mVersionBuf.data();
    mInfo.game.logo = mLogoBuf.data();
    mInfo.game.menuBackground = mMenuBackgroundBuf.data();
    mInfo.game.startScene = mStartSceneBuf.data();
    mInfo.store.nqSave("GAME.INFO"_sv, mInfo.game);
  }

  render::AspectRatio m1x1{1, 1};
  render::AspectRatio m4x3{4, 3};
  bool mShowBackground = false;
  render::Texture mBackground;

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

SubState *gameInfoEditor(EditorInfo &info) {
  return new GameInfoEditorSubState(info);
}

} // namespace sigmoid
