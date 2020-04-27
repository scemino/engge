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
    auto pLogger = std::make_shared<ng::Logger>();
    ng::Locator<ng::Logger>::set(pLogger);

    auto pResManager = std::make_shared<ng::ResourceManager>();
    ng::Locator<ng::ResourceManager>::set(pResManager);

    auto pSettings = std::make_shared<ng::EngineSettings>();
    ng::Locator<ng::EngineSettings>::set(pSettings);

    auto pSoundManager = std::make_shared<ng::SoundManager>();
    ng::Locator<ng::SoundManager>::set(pSoundManager);

    auto pPreferences = std::make_shared<ng::Preferences>();
    ng::Locator<ng::Preferences>::set(pPreferences);

    ng::Locator<ng::TextDatabase>::set(std::make_shared<ng::TextDatabase>());

    auto game = std::make_unique<ng::Game>();
    auto scriptEngine = std::make_unique<ng::ScriptEngine>();
    auto engine = std::make_unique<ng::Engine>();
    game->setEngine(engine.get());
    scriptEngine->setEngine(*engine);

    game->getInputEventHandlers().push_back(std::make_unique<ng::DefaultInputEventHandler>(*engine, game->getWindow()));
    game->run();

    ng::Locator<ng::SoundManager>::reset();
  }
  catch (std::exception &e) {
    ng::error("Sorry, an error occurred: {}", e.what());
  }
  catch (...) {
    ng::error("Sorry, an error occurred");
  }

  return 0;
}
