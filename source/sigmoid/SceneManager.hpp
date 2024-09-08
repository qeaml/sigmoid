#pragma once

/*
SceneManager.hpp
----------------
Allows for game scenes to be managed. Provides a 'Scene Manager' window.
*/

#include <nwge/common/slice.hpp>
#include <nwge/common/string.hpp>
#include <nwge/data/store.hpp>

namespace sigmoid {

class SceneManager final {
public:
  SceneManager(const nwge::StringView &gameName);
  void discover();
  [[nodiscard]]
  nwge::ArrayView<const nwge::String<>> scenes() const;
  void window(struct EditorInfo &info);

private:
  static constexpr usize cBufSize = 40;

  nwge::data::Store mStore;
  nwge::String<> mGameName;
  nwge::Slice<nwge::String<>> mScenes{4};
  ssize mSelectedScene = -1;
};

} // namespace sigmoid
