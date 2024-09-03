#include "states.hpp"
#include <nwge/common/maybe.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class StorySceneSubState final: public SubState {
public:
  StorySceneSubState(SceneStateData &data)
    : mData(data)
  {}

  void render() const override {
    render::clear();
    render::color();
    mData.font.cursor({0.1, 0.1, 0.1}, 0.05)
     << mData.scene.title() << '\n'
     << s32(mStory.actors.size()) << " actors\n"_sv
     << s32(mStory.sprites.size()) << " sprites\n"_sv
     << s32(mStory.commands.size()) << " commands\n"_sv
     << s32(mStory.bgImages.size()) << " bg images\n"_sv
     << s32(mStory.musicTracks.size()) << " music tracks\n"_sv
     << render::Cursor::cSentinel;
  }

private:
  SceneStateData &mData;
  const StoryScene &mStory = mData.scene.story();
};

SubState *storyScene(SceneStateData &data) {
  return new StorySceneSubState(data);
}

} // namespace sigmoid
