#include "states.hpp"
#include "Game.hpp"
#include <nwge/common/maybe.hpp>

using namespace nwge;

namespace sigmoid {

class LaunchState final: public State {
public:
  LaunchState(const StringView &gameName)
    : mName(gameName), mGame(gameName)
  {}

  bool preload() override {
    if(mName.empty()) {
      dialog::error("LaunchState"_sv, "No game selected."_sv);
      return false;
    }
    mGame.preload();
    return true;
  }

  bool init() override {
    swapStatePtr(gameMenu(std::move(mGame)));
    return true;
  }

private:
  String<> mName;
  Game mGame;
};

State *launch(const StringView &gameName) {
  return new LaunchState(gameName);
}

} // namespace sigmoid
