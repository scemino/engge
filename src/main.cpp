#include <memory>
#include "Game.h"
#include "GGEngine.h"
#include "ScriptEngine.h"
#include "PanInputEventHandler.h"

int main()
{
    gg::GGEngineSettings settings("./resources/");
    auto engine = std::make_unique<gg::GGEngine>(settings);

    auto game = std::make_unique<gg::Game>(*engine);
    auto scriptEngine = std::make_unique<gg::ScriptEngine>(*engine);
    scriptEngine->executeScript("test.nut");

    game->getInputEventHandlers().push_back(std::make_unique<gg::PanInputEventHandler>(*engine.get(), game->getWindow()));
    game->getInputEventHandlers().push_back(std::make_unique<gg::EngineShortcutsInputEventHandler>(*engine.get(), game->getWindow()));
    game->run();

    return 0;
}
