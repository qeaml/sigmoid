#include "states.hpp"
#include "imgui/imgui.hpp"
#include <nwge/engine.hpp>
#include <nwge/cli/cli.h>
#include <nwge/gui.hpp>

using namespace nwge;
using namespace sigmoid;

s32 main(s32 argc, CStr *argv) {
  cli::parse(argc, argv);

  ImGui::SetAllocatorFunctions(
    [](usize size, AnyPtr){
      return nwgeAlloc(size);
    },
    [](AnyPtr ptr, AnyPtr){
      nwgeFree(ptr);
    },
    nullptr
  );
  ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(guiContext()));

  StringView gameName;
  if(cli::posC() >= 1) {
    gameName = cli::pos(0);
  }
  startPtr(launch(gameName), {
    .appName = "Sigmoid Engine"_sv,
  });
  return 0;
}
