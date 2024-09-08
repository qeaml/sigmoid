#pragma once

/*
AssetManager.hpp
----------------
Allows for assets stored in the game's data store to be loaded and
otherwise managed. Provides an 'Asset Manager' window.
*/

#include <nwge/common/slice.hpp>
#include <nwge/common/string.hpp>
#include <nwge/data/store.hpp>

namespace sigmoid {

class AssetManager final {
public:
  AssetManager(const nwge::StringView &gameName);
  void discover();
  [[nodiscard]]
  nwge::ArrayView<const nwge::String<>> assets() const;
  void window();

private:
  nwge::data::Store mStore;
  nwge::Slice<nwge::String<>> mAssets{4};
  ssize mSelectedAsset = -1;
};

} // namespace sigmoid
