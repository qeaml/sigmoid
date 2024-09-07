#include "states.hpp"
#include "imgui/imgui.hpp"
#include <nwge/data/file.hpp>
#include <nwge/data/store.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class EditorGameMenuState final: public State {
public:
  EditorGameMenuState(const nwge::StringView &gameName)
    : mStore(gameName), mName(gameName)
  {}

  bool preload() override {
    nqLoadGameInfo();
    return true;
  }

  bool init() override {
    findFiles();
    return true;
  }

  bool on(Event &evt) override {
    switch(evt.type) {
    case Event::PostLoad:
      if(mGameInfo.present()) {
        safeCopyString(mGameInfo->title, mTitleBuf);
        safeCopyString(mGameInfo->author, mAuthorBuf);
        safeCopyString(mGameInfo->description, mDescriptionBuf);
        safeCopyString(mGameInfo->version, mVersionBuf);
        safeCopyString(mGameInfo->logo, mLogoBuf);
        safeCopyString(mGameInfo->menuBackground, mMenuBackgroundBuf);
        safeCopyString(mGameInfo->startScene, mStartSceneBuf);
        mGameInfo.clear();
      }
      break;
    default:
      break;
    }
    return true;
  }

  bool tick([[maybe_unused]] f32 delta) override {
    gameInfoWindow();
    sceneListWindow();
    assetListWindow();
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
  }

private:
  data::Store mStore;
  String<> mName;
  Maybe<Game> mGameInfo;
  Slice<String<>> mScenes{4};
  Slice<String<>> mAssets{4};

  render::Texture mAssetTexture;

  void findFiles() {
    mScenes.clear();
    mAssets.clear();
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
      ScratchArray<char> name = filename;
      toUpper(name.view());
      mAssets.push(StringView(name.view()));
    }
  }

  void sceneListWindow() {
    if(!ImGui::Begin("Scene List", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return;
    }

    if(ImGui::BeginListBox("##scenes")) {
      for(auto &scene: mScenes) {
        ImGui::Selectable(scene.begin());
      }
      ImGui::EndListBox();
    }

    static std::array<char, cBufSize> sNameBuf{};
    ImGui::InputText("Scene Name", sNameBuf.data(), cBufSize);
    if(ImGui::Button("Create Story Scene")) {
      swapStatePtr(editorStoryScene(mName, sNameBuf.begin()));
    }
    ImGui::SameLine();
    if(ImGui::Button("Create Field Scene")) {
      // TODO
    }

    ImGui::End();
  }

  void assetListWindow() {
    if(!ImGui::Begin("Asset List", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return;
    }

    if(ImGui::BeginListBox("##assets")) {
      for(auto &asset: mAssets) {
        ImGui::Selectable(asset.begin());
      }
      ImGui::EndListBox();
    }

    if(ImGui::Button("Import")) {
      auto maybePath = dialog::FilePicker{}
        .title("Import asset"_sv)
        .openOne();
      if(maybePath.present()) {
        ScratchArray<char> name = maybePath->filename();
        toUpper(name.view());
        data::nqCopy(*maybePath, mStore.path().join({name.view()}));
        mAssets.push(StringView(name.view()));
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
