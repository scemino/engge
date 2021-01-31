#pragma once
#include <Engine/AchievementManager.hpp>
#include "engge/Audio/SoundManager.hpp"
#include "engge/Input/CommandManager.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Engine/TextDatabase.hpp"
#include "Locator.hpp"
#include "Logger.hpp"
#include "engge/Util/RandomNumberGenerator.hpp"

namespace ng {
class Services final {
public:
  static void init() {
    ng::Locator<ng::Logger>::create();
    ng::info("Init services");
    ng::Locator<ng::RandomNumberGenerator>::create();
    ng::Locator<ng::CommandManager>::create();
    ng::Locator<ng::AchievementManager>::create();
    ng::Locator<ng::Preferences>::create();
    ng::Locator<ng::EngineSettings>::create().loadPacks();
    ng::Locator<ng::EntityManager>::create();
    ng::Locator<ng::SoundManager>::create();
    ng::Locator<ng::TextDatabase>::create();
    ng::Locator<ng::ResourceManager>::create();
  }
};
}