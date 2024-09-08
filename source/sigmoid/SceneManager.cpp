#include "SceneManager.hpp"
#include "imgui/imgui.hpp"
#include "states.hpp"
#include <nwge/common/cast.hpp>
#include <nwge/common/slice.hpp>
#include <nwge/data/file.hpp>
#include <nwge/dialog.hpp>

using namespace nwge;

namespace sigmoid {

SceneManager::SceneManager(const StringView &gameName)
  : mStore(gameName), mGameName(gameName)
{}

void SceneManager::discover() {
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

ArrayView<const String<>> SceneManager::scenes() const {
  return mScenes.view();
}

void SceneManager::window(EditorInfo &info) {
  if(!ImGui::Begin("Scene Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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
      setEditorSubState(sceneEditor(info, scene));
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
      setEditorSubState(sceneEditor(info, sNameBuf.begin()));
    }
    ImGui::SameLine();
    if(ImGui::Button("Create Field Scene")) {
      // TODO
    }
  }

  ImGui::End();
}

} // namespace sigmoid
