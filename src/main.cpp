#include <memory>
#include "Game.h"
#include "GGEngine.h"
#include "ScriptEngine.h"
#include "PanInputEventHandler.h"

int main()
{
    gg::GGEngineSettings settings("./resources/");
    gg::GGEngine engine(settings);

    gg::Game game(engine);

    gg::ScriptEngine scriptEngine(engine);
    scriptEngine.executeScript("test.nut");

    game.getInputEventHandlers().push_back(std::make_unique<gg::PanInputEventHandler>(engine, game.getWindow()));
    game.getInputEventHandlers().push_back(std::make_unique<gg::EngineShortcutsInputEventHandler>(engine, game.getWindow()));
    game.run();

    return 0;
}
