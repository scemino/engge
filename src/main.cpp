#include <filesystem>
#include <memory>
#include "Input/InputMappings.hpp"
#include "Input/CommandManager.hpp"
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
#include "Dialog/DialogScriptAbstract.hpp"
#include "Dialog/_AstDump.hpp"
#include "Parsers/SavegameManager.hpp"
namespace fs = std::filesystem;

int main(int argc, char **argv) {
  if (argc == 2) {
    auto filename = argv[1];
    auto filepath = fs::path(filename);
    auto ext = filepath.extension();
    if (ext == ".byack") {
      ng::_AstDump::dump(filename);
    } else if (ext == ".save") {
      ng::GGPackValue hash;
      ng::SavegameManager::loadGame(filename, hash);
      std::cout << hash;
    }
    return 0;
  }

  try {
    ng::Locator<ng::Logger>::create();
    ng::info("Init services");
    ng::Locator<ng::CommandManager>::create();
    ng::Locator<ng::EngineSettings>::create().loadPacks();
    ng::Locator<ng::ResourceManager>::create();
    ng::Locator<ng::SoundManager>::create();
    ng::Locator<ng::Preferences>::create();
    ng::Locator<ng::TextDatabase>::create();
    auto &scriptEngine = ng::Locator<ng::ScriptEngine>::create();
    auto &engine = ng::Locator<ng::Engine>::create();
    auto &game = ng::Locator<ng::Game>::create();
    game.setEngine(&engine);
    scriptEngine.setEngine(engine);

    game.getInputEventHandlers().push_back(std::make_unique<ng::DefaultInputEventHandler>(engine, game.getWindow()));
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
