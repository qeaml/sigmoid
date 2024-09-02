#include "states.hpp"
#include <nwge/engine.hpp>
#include <nwge/cli/cli.h>

using namespace nwge;
using namespace sigmoid;

s32 main(s32 argc, CStr *argv) {
  cli::parse(argc, argv);

  StringView gameName;
  if(cli::posC() >= 1) {
    gameName = cli::pos(0);
  }
  startPtr(launch(gameName), {
    .appName = "Sigmoid Engine"_sv,
  });
  return 0;
}
