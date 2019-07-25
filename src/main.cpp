#include <memory>
#include "Game.h"
#include "Engine.h"
#include "ScriptEngine.h"
#include "PanInputEventHandler.h"
#include "Dialog/_AstDump.h"

int main(int argc, char **argv)
{
    ng::EngineSettings settings;
    if (argc == 2)
    {
        auto filename = argv[1];
        std::cout << argc << std::endl;
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
        scriptEngine->executeScript("test.nut");
        // scriptEngine->executeBootScript();

        game->getInputEventHandlers().push_back(std::make_unique<ng::PanInputEventHandler>(*engine, game->getWindow()));
        game->getInputEventHandlers().push_back(std::make_unique<ng::EngineShortcutsInputEventHandler>(*engine));
        game->run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Sorry, an error occured: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Sorry, an error occured" << std::endl;
    }

    return 0;
}
