#include <filesystem>
#include <memory>
#include "Game.hpp"
#include "Engine/Engine.hpp"
#include "Engine/EngineSettings.hpp"
#include "Engine/TextDatabase.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Engine/Preferences.hpp"
#include "Engine/ResourceManager.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Audio/SoundDefinition.hpp"
#include "Audio/SoundId.hpp"
#include "Audio/SoundManager.hpp"
#include "Input/DefaultInputEventHandler.hpp"
#include "Dialog/_AstDump.hpp"

int main(int argc, char **argv) {
  if (argc == 2) {
    auto filename = argv[1];
    ng::_AstDump::dump(filename);
    return 0;
  }

  try {
    ng::Locator<ng::Logger>::create();
    ng::info("Init services");
    ng::Locator<ng::EngineSettings>::create().loadPacks();
    ng::Locator<ng::ResourceManager>::create();
    ng::Locator<ng::SoundManager>::create();
    ng::Locator<ng::Preferences>::create();
    ng::Locator<ng::TextDatabase>::create();
    auto& scriptEngine = ng::Locator<ng::ScriptEngine>::create();
    auto& engine = ng::Locator<ng::Engine>::create();
    auto& game = ng::Locator<ng::Game>::create();
    game.setEngine(&engine);
    scriptEngine.setEngine(engine);

    game.getInputEventHandlers().push_back(std::make_unique<ng::DefaultInputEventHandler>(engine, game.getWindow()));

    ng::info("Start game");
    game.run();
  }
  catch (std::exception &e) {
    ng::error("Sorry, an error occurred: {}", e.what());
  }
  catch (...) {
    ng::error("Sorry, an error occurred");
  }

  return 0;
}
