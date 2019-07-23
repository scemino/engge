#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Engine.h"
#include "EngineSettings.h"
#include "ScriptEngine.h"

using namespace Catch::Matchers;

TEST_CASE("general pack", "[general]")
{
    ng::ScriptEngine scriptEngine;
    ng::EngineSettings settings;
    ng::Engine engine(settings);
    scriptEngine.setEngine(engine);
    srand(0);
    SECTION("random")
    {
        auto result = engine.executeDollar("\"\"+random(2,5);");
        REQUIRE_THAT(result, Equals("4"));
    }
    SECTION("randomOdds")
    {
        auto result = engine.executeDollar("\"\"+randomOdds(2);");
        REQUIRE_THAT(result, Equals("true"));
        result = engine.executeDollar("\"\"+randomOdds(0);");
        REQUIRE_THAT(result, Equals("false"));
    }
    SECTION("screenSize")
    {
        sf::RenderWindow w;
        engine.setWindow(w);
        auto result = engine.executeDollar("\"x=\"+screenSize().x+\" y=\"+screenSize().y;");
        REQUIRE_THAT(result, Equals("x=1000 y=1000"));
    }
    SECTION("strfind")
    {
        auto result = engine.executeDollar("\"\"+strfind(\"hello *world*\",\"*world*\");");
        REQUIRE_THAT(result, Equals("6"));
    }
    SECTION("strreplace")
    {
        auto result = engine.executeDollar("strreplace(\"hello *world*\",\"*world*\",\"everybody\");");
        REQUIRE_THAT(result, Equals("hello everybody"));
    }
}