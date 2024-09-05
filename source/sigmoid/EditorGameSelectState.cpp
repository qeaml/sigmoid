#include "states.hpp"
#include "imgui/imgui.hpp"
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

class EditorGameSelectState final: public State {
public:
  bool tick(f32 delta) override {
    if(!ImGui::Begin("Game Selection", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::End();
      return true;
    }

    ImGui::InputText("##gameName", mNameBuf.data(), cNameBufSize);
    ImGui::SameLine();
    if(ImGui::Button("Edit")) {
      swapStatePtr(editorGameMenu(mNameBuf.data()));
    }

    ImGui::End();
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
  }

private:
  static constexpr usize cNameBufSize = 40;
  std::array<char, cNameBufSize> mNameBuf{};
};

State *editorGameSelect() {
  return new EditorGameSelectState;
}

} // namespace sigmoid
