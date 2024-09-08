#include "AssetManager.hpp"
#include "imgui/imgui.hpp"
#include <nwge/common/cast.hpp>
#include <nwge/common/slice.hpp>
#include <nwge/data/file.hpp>
#include <nwge/dialog.hpp>

using namespace nwge;

namespace sigmoid {

AssetManager::AssetManager(const StringView &gameName)
  : mStore(gameName)
{}

void AssetManager::discover() {
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
      continue;
    }
    ScratchArray<char> name = filename;
    toUpper(name.view());
    mAssets.push(StringView(name.view()));
  }
}

ArrayView<const String<>> AssetManager::assets() const {
  return mAssets.view();
}

void AssetManager::window() {
  if(!ImGui::Begin("Asset Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::End();
    return;
  }

  if(ImGui::BeginListBox("##assets")) {
    for(usize i = 0; i < mAssets.size(); ++i) {
      auto &asset = mAssets[i];
      auto idx = saturate_cast<ssize>(i);
      if(ImGui::Selectable(asset.begin(), mSelectedAsset == idx)) {
        mSelectedAsset = idx;
      }
    }
    ImGui::EndListBox();
  }

  if(mSelectedAsset != -1) {
    ImGui::Text("Selected: %s", mAssets[mSelectedAsset].begin());
    if(ImGui::Button("Deselect")) {
      mSelectedAsset = -1;
    }
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
      auto selected = mAssets[mSelectedAsset].view();
      data::nqDelete(mStore.path().join({selected}));
      Slice<String<>> newAssets{mAssets.size() - 1};
      for(const auto &asset: mAssets) {
        if(asset.view() != selected) {
          newAssets.push(asset);
        }
      }
      mAssets = {newAssets.view()};
      mSelectedAsset = -1;
    }
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

} // namespace sigmoid
