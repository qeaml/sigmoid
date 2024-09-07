#include "states.hpp"
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class SceneState final: public State {
public:
  SceneState(Game &&game, const StringView &sceneName)
    : mGame(std::move(game)), mScene(sceneName)
  {}

  bool preload() override {
    mBundle
      .load({"sigmoid.bndl"_sv})
      .nqFont("INTER.CFN"_sv, mFont);

    const auto &name = mScene.name;
    if(name.empty()) {
      dialog::error("SceneState"_sv, "No scene selected."_sv);
      return false;
    }
    ScratchArray<char> filename = ScratchString::formatted("{}.scn", name);
    toUpper(filename.view());
    mGame.bundle
      .nqCustom(filename.view(), mScene);
    return true;
  }

  bool init() override {
    switch(mScene.type) {
    case SceneField:
      dialog::info("SceneState"_sv,
        "Field scene not implemented yet."_sv);
      return false;
    case SceneStory:
      pushSubStatePtr(storyScene(mData));
      break;
    default:
      NWGE_UNREACHABLE("Invalid scene type");
    }

    return true;
  }

  bool tick([[maybe_unused]] f32 delta) override {
    /*
    We only reach here once the appropriate scene SubState has been popped.
    Return to menu.
    */
    swapStatePtr(gameMenu(std::move(mGame)));
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
  }

private:
  data::Bundle mBundle;

  render::Font mFont;
  Game mGame;
  Scene mScene;
  SceneStateData mData{mGame, mScene, mFont};
};

State *scene(Game &&game, const StringView &sceneName) {
  return new SceneState(std::move(game), sceneName);
}

} // namespace sigmoid
