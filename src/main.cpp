#include <memory>
#include "Game.h"
#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "System/Locator.h"
#include "System/Logger.h"
#include "Engine/Preferences.h"
#include "Engine/ResourceManager.h"
#include "Scripting/ScriptEngine.h"
#include "Audio/SoundDefinition.h"
#include "Audio/SoundId.h"
#include "Audio/SoundManager.h"
#include "Input/DefaultInputEventHandler.h"
#include "Dialog/_AstDump.h"

int main(int argc, char **argv)
{
    try
    {
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

        if (argc == 2)
        {
            auto filename = argv[1];
            ng::_AstDump::dump(filename);
            return 0;
        }

        auto game = std::make_unique<ng::Game>();
        auto scriptEngine = std::make_unique<ng::ScriptEngine>();
        auto engine = std::make_unique<ng::Engine>();
        game->setEngine(engine.get());
        scriptEngine->setEngine(*engine);

        std::ifstream is("test.nut");
        if (is.is_open())
        {
            ng::info("execute test.nut");
            scriptEngine->executeScript("test.nut");
        }
        else
        {
            ng::info("execute boot script");
            scriptEngine->executeBootScript();
        }

        game->getInputEventHandlers().push_back(std::make_unique<ng::DefaultInputEventHandler>(*engine, game->getWindow()));
        game->run();

        ng::Locator<ng::SoundManager>::reset();
    }
    catch (std::exception &e)
    {
        ng::error("Sorry, an error occurred: {}", e.what());
    }
    catch (...)
    {
        ng::error("Sorry, an error occurred");
    }

    return 0;
}
