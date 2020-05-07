#include <filesystem>
#include <memory>
#include <Parsers/SavegameManager.hpp>
#include <Dialog/DialogManager.hpp>
#include <Dialog/ExpressionVisitor.hpp>
#include <Dialog/DialogPlayer.hpp>
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
namespace fs = std::filesystem;

namespace ng {

class DialogScript : public DialogScriptAbstract {
public:
  std::function<bool()> pause(sf::Time time) {
    std::cout << "pause " << time.asSeconds() << std::endl;
    return []() { return true; };
  }
  std::function<bool()> say(const std::string &actor, const std::string &text) {
    std::cout << actor << ": " << text << std::endl;
    return []() { return true; };
  }
  void shutup() { std::cout << "shutup" << std::endl; }
  std::function<bool()> waitFor(const std::string &actor) {
    std::cout << "waitFor " << actor << std::endl;
    return []() {
      return true;
    };
  }
  std::function<bool()> waitWhile(const std::string &condition) {
    std::cout << "waitWhile " << condition << std::endl;
    return []() {
      return true;
    };
  }
  void execute(const std::string &code) {
    std::cout << "execute " << code << std::endl;
  }
  bool executeCondition(const std::string &condition) const {
    std::cout << "executeCondition " << condition << std::endl;
    return true;
  }
};
}

int main(int argc, char **argv) {
  if (argc == 2) {

    auto filename = argv[1];
    auto filepath = fs::path(filename);
    auto ext = filepath.extension();
    if (ext.empty()) {
      ng::Locator<ng::Logger>::create();
      ng::Locator<ng::EngineSettings>::create().loadPacks();
      ng::DialogScript ds;
      ng::DialogPlayer dp(ds);
      sf::Clock clock;
      dp.start("ray", filename, "init");
      while (dp.getState() != ng::DialogManagerState::None) {
        dp.update();
      }

      dp.start("ray", filename, "start");
      while (dp.getState() != ng::DialogManagerState::None) {
        if (dp.getState() == ng::DialogManagerState::WaitingForChoice) {
          const auto &choices = dp.getChoices();
          int i = 1;
          for (const auto &choice : choices) {
            if (!choice)
              continue;
            auto pChoice = dynamic_cast<ng::Ast::Choice *>(choice->expression.get());
            std::cout << (i++) << " " << pChoice->text << std::endl;
          }
          int choice;
          std::cin >> choice;
          dp.choose(choice);
        }
        dp.update();
      }
      //ng::_AstDump::dump(filename);
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

    ng::info("Start game");
    game.run();
  }
  catch (std::exception &e) {
    ng::error("Sorry, an error occurred: {}", e.what());
  }
  catch (...) {
    ng::error("Sorry, an error occurred");
  }

  return 0;
}
