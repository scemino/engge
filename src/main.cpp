#include <memory>
#include "engge/Engine/Engine.hpp"
#include "engge/Game.hpp"
#include "engge/Input/InputMappings.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/System/Services.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "Util/Dumper.hpp"

int main(int argc, char **argv) {
  try {
    if (argc == 2) {
      Dumper::dump(argv[1]);
      return 0;
    }

    ng::Services::init();
    auto &scriptEngine = ng::Locator<ng::ScriptEngine>::create();
    auto &engine = ng::Locator<ng::Engine>::create();
    auto &game = ng::Locator<ng::Game>::create();
    game.setEngine(&engine);
    scriptEngine.setEngine(engine);

    ng::InputMappings::registerMappings();
    ng::info("Start game");
    game.run();
  }
  catch (std::exception &e) {
    ng::error("Sorry, an error occurred: {}", e.what());
    return 1;
  }
  catch (...) {
    ng::error("Sorry, an error occurred");
    return 2;
  }
  return 0;
}
