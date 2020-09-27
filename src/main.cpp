#include <memory>
#include "Engine/Engine.hpp"
#include "Game.hpp"
#include "Input/InputMappings.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "System/Services.hpp"
#include "Scripting/ScriptEngine.hpp"
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
