#include <string>
#include "Input/InputConstants.hpp"
#include "System/Locator.hpp"
#include "Engine/Preferences.hpp"
#include "Input/CommandManager.hpp"
#include "Engine/EngineCommands.hpp"
#include "Input/InputMappings.hpp"

namespace ng {

static InputConstants toKey(const std::string &keyText) {
  if (keyText.length() == 1) {
    return static_cast<ng::InputConstants>(keyText[0]);
  }
  return ng::InputConstants::NONE;
}

template<typename T>
static InputConstants toKey(const std::string &name, T value) {
  const auto &preferences = ng::Locator<ng::Preferences>::get();
  return toKey(preferences.getUserPreference(name, value));
}

void InputMappings::registerMappings() {
  auto keySkip = toKey(ng::PreferenceNames::KeySkipText, ng::PreferenceDefaultValues::KeySkipText);
  auto keySelect1 = toKey(ng::PreferenceNames::KeySelect1, ng::PreferenceDefaultValues::KeySelect1);
  auto keySelect2 = toKey(ng::PreferenceNames::KeySelect2, ng::PreferenceDefaultValues::KeySelect2);
  auto keySelect3 = toKey(ng::PreferenceNames::KeySelect3, ng::PreferenceDefaultValues::KeySelect3);
  auto keySelect4 = toKey(ng::PreferenceNames::KeySelect4, ng::PreferenceDefaultValues::KeySelect4);
  auto keySelect5 = toKey(ng::PreferenceNames::KeySelect5, ng::PreferenceDefaultValues::KeySelect5);
  auto keySelect6 = toKey(ng::PreferenceNames::KeySelect6, ng::PreferenceDefaultValues::KeySelect6);
  auto keySelectPrev = toKey(ng::PreferenceNames::KeySelectPrev, ng::PreferenceDefaultValues::KeySelectPrev);
  auto keySelectNext = toKey(ng::PreferenceNames::KeySelectNext, ng::PreferenceDefaultValues::KeySelectNext);

  ng::Locator<ng::CommandManager>::get().registerInputBindings(
      {
          {{ng::InputConstants::KEY_SPACE}, ng::EngineCommands::PauseGame},
          {{ng::InputConstants::KEY_ESCAPE}, ng::EngineCommands::SkipCutscene},
          {{keySelect1}, ng::EngineCommands::SelectActor1},
          {{keySelect2}, ng::EngineCommands::SelectActor2},
          {{keySelect3}, ng::EngineCommands::SelectActor3},
          {{keySelect4}, ng::EngineCommands::SelectActor4},
          {{keySelect5}, ng::EngineCommands::SelectActor5},
          {{keySelect6}, ng::EngineCommands::SelectActor6},
          {{keySelectPrev}, ng::EngineCommands::SelectPreviousActor},
          {{keySelectNext}, ng::EngineCommands::SelectNextActor},
          {{keySkip}, ng::EngineCommands::SkipText},
          {{MetaKeys::Control, ng::InputConstants::KEY_O}, ng::EngineCommands::ShowOptions},
          {{MetaKeys::Control, ng::InputConstants::KEY_U}, ng::EngineCommands::ToggleHud},
          {{ng::InputConstants::KEY_TAB}, ng::EngineCommands::ShowHotspots}
      });
}
}