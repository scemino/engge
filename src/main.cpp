#include <memory>
#include "Game.h"
#include "Engine.h"
#include "ScriptEngine.h"
#include "PanInputEventHandler.h"
#include "Dialog/_AstDump.h"

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        auto filename = argv[1];
        std::cout << argc << std::endl;
        ng::_AstDump::dump(filename);
        return 0;
    }

    ng::EngineSettings settings("./resources/");
    auto engine = std::make_unique<ng::Engine>(settings);

    auto game = std::make_unique<ng::Game>(*engine);
    auto scriptEngine = std::make_unique<ng::ScriptEngine>(*engine);
    scriptEngine->executeScript("test.nut");

    game->getInputEventHandlers().push_back(std::make_unique<ng::PanInputEventHandler>(*engine.get(), game->getWindow()));
    game->getInputEventHandlers().push_back(std::make_unique<ng::EngineShortcutsInputEventHandler>(*engine.get(), game->getWindow()));
    game->run();

    return 0;
}
