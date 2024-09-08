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
    : mStore(gameName), mName(gameName), mAssets(gameName)
  {}

  bool preload() override {
    nqLoadGameInfo();
    mAssets.discover();
    return true;
  }

  bool init() override {
    copyGameInfo();
    findFiles();
    if(mMenuBackgroundBuf[0] != 0) {
      mShowBackground = true;
      mStore.nqLoad(mMenuBackgroundBuf.begin(), mBackground);
    }
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
    sceneListWindow();
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
      if(mGameInfo->menuBackground[0] == 0) {
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

  Slice<String<>> mScenes{4};
  ssize mSelectedScene = -1;
  AssetManager mAssets;

  bool mShowBackground = false;
  render::Texture mBackground;

  void findFiles() {
    mScenes.clear();
    for(auto iter = mStore.path().iterate(); iter; ++iter) {
      auto path = *iter;
      if(path.isDir()) {
        continue;
      }
      const auto &filename = path.filename();
      if(filename.equalsIgnoreCase("GAME.INFO"_sv)) {
        continue;
      }
      if(filename.endsWithIgnoreCase(".SCN"_sv)) {
        ScratchArray<char> name = filename.trimSuffixIgnoreCase(".SCN"_sv);
        toUpper(name.view());
        mScenes.push(StringView(name.view()));
        continue;
      }
    }
  }

  void sceneListWindow() {
    if(!ImGui::Begin("Scene List", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return;
    }

    if(ImGui::BeginListBox("##scenes")) {
      for(usize i = 0; i < mScenes.size(); ++i) {
        auto &scene = mScenes[i];
        auto idx = saturate_cast<ssize>(i);
        if(ImGui::Selectable(scene.begin(), mSelectedScene == idx)) {
          mSelectedScene = idx;
        }
      }
      ImGui::EndListBox();
    }

    if(mSelectedScene != -1) {
      const auto &scene = mScenes[mSelectedScene];
      ImGui::Text("Selected: %s", scene.begin());
      if(ImGui::Button("Edit")) {
        swapStatePtr(editorStoryScene(mName, scene.view()));
      }
      ImGui::SameLine();
      if(ImGui::Button("Deselect")) {
        mSelectedScene = -1;
      }
      ImGui::SameLine();
      if(ImGui::Button("Delete")) {
        auto selected = scene.view();
        data::nqDelete(mStore.path().join({selected}).ext("SCN"_sv));
        Slice<String<>> newScenes{mScenes.size() - 1};
        for(const auto &asset: mScenes) {
          if(asset.view() != selected) {
            newScenes.push(asset);
          }
        }
        mScenes = {newScenes.view()};
        mSelectedScene = -1;
      }
    } else {
      static std::array<char, cBufSize> sNameBuf{};
      ImGui::InputText("Scene Name", sNameBuf.data(), cBufSize);
      if(ImGui::Button("Create Story Scene")) {
        swapStatePtr(editorStoryScene(mName, sNameBuf.begin()));
      }
      ImGui::SameLine();
      if(ImGui::Button("Create Field Scene")) {
        // TODO
      }
    }

    ImGui::End();
  }

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
