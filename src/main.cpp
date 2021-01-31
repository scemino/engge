#include "Engine/AchievementManager.hpp"
#include "engge/EnggeApplication.hpp"

int main(int, char *[]) {
  ng::EnggeApplication app;
  try {
    app.run();
    ng::Locator<ng::Engine>::reset();
  }
  catch (std::exception &e) {
    app.showMessageBox("Critical Error!", e.what(), ngf::MessageBoxType::Warning);
    return 1;
  }
  catch (...) {
    ng::error("Sorry, an error occurred");
    return 2;
  }
  return EXIT_SUCCESS;
}
