#pragma once
#include "Audio/SoundManager.hpp"
#include "Input/CommandManager.hpp"
#include "Engine/EngineSettings.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/Preferences.hpp"
#include "Engine/TextDatabase.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Util/RandomNumberGenerator.hpp"

namespace ng {
class Services final {
public:
  static void init() {
    ng::Locator<ng::Logger>::create();
    ng::info("Init services");
    ng::Locator<ng::RandomNumberGenerator>::create();
    ng::Locator<ng::CommandManager>::create();
    ng::Locator<ng::Preferences>::create();
    ng::Locator<ng::EngineSettings>::create().loadPacks();
    ng::Locator<ng::EntityManager>::create();
    ng::Locator<ng::SoundManager>::create();
    ng::Locator<ng::TextDatabase>::create();
    ng::Locator<ng::ResourceManager>::create();
  }
};
}