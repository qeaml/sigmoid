#include "AssetManager.hpp"
#include "SceneManager.hpp"
#include "imgui/imgui.hpp"
#include "states.hpp"
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class EditorState final: public State {
public:
  EditorState(const nwge::StringView &gameName)
    : mInfo(
        gameName,
        {gameName},
        {gameName},
        {gameName},
        {gameName})
  {}

  bool preload() override {
    mInfo.store.nqLoad("GAME.INFO"_sv, mInfo.game);
    mInfo.scenes.discover();
    mInfo.assets.discover();
    return true;
  }

  bool init() override {
    ScratchString title = ScratchString::formatted(
      "Sigmoid Engine Editor - {}",
      mInfo.game.title
    );
    setEditorSubState(gameInfoEditor(mInfo));
    return true;
  }

  bool tick([[maybe_unused]] f32 delta) override {
    mInfo.scenes.window(mInfo);
    mInfo.assets.window();
    return true;
  }

  void render() const override {
    render::clear();
  }

private:
  EditorInfo mInfo;
};

State *editorState(const nwge::StringView &gameName) {
  return new EditorState(gameName);
}

} // namespace sigmoid
