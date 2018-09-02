#include <memory>
#include "Game.h"
#include "GGEngine.h"
#include "ScriptEngine.h"
#include "PanInputEventHandler.h"

int main()
{
    gg::GGEngineSettings settings("./resources/");
    gg::GGEngine engine(settings);

    gg::ScriptEngine scriptEngine(engine);
    scriptEngine.executeScript("test.nut");

    gg::Game game(engine);
    game.getInputEventHandlers().push_back(std::make_unique<gg::PanInputEventHandler>(game.getWindow()));
    game.getInputEventHandlers().push_back(std::make_unique<gg::EngineShortcutsInputEventHandler>(engine, game.getWindow()));
    game.run();

    return 0;
}
