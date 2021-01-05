#include "engge/EnggeApplication.hpp"

int main(int, char **) {
  try {
    ng::EnggeApplication app;
    app.run();
    ng::Locator<ng::Engine>::reset();
  }
  catch (std::exception &e) {
    ng::error("Sorry, an error occurred: {}", e.what());
    return 1;
  }
  catch (...) {
    ng::error("Sorry, an error occurred");
    return 2;
  }
  return EXIT_SUCCESS;
}
