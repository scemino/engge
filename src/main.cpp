#include <memory>
#include "Game.h"
#include "Engine.h"
#include "Locator.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "ScriptEngine.h"
#include "DefaultInputEventHandler.h"
#include "Dialog/_AstDump.h"

int main(int argc, char **argv)
{
    auto pLogger = std::make_unique<ng::Logger>();
    ng::Locator::registerService(pLogger.get());

    auto pResManager = std::make_unique<ng::ResourceManager>();
    ng::Locator::registerService(pResManager.get());

    ng::EngineSettings settings;
    if (argc == 2)
    {
        auto filename = argv[1];
        ng::_AstDump::dump(settings, filename);
        return 0;
    }

	auto game = std::make_unique<ng::Game>();
	auto scriptEngine = std::make_unique<ng::ScriptEngine>();
    auto engine = std::make_unique<ng::Engine>(settings);
    game->setEngine(engine.get());
    scriptEngine->setEngine(*engine);
    try
    {
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
