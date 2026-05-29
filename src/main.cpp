#include <filesystem>
#include <iostream>
#include <wtts/interactiveCLI.hpp>

#include <ncurses.h>

void err(std::string const &msg) {
  def_prog_mode();
  endwin();
  std::cerr << "ERROR: " << msg << std::endl;
  reset_prog_mode();
}

int main(int, char **argv) {
  try {
    std::filesystem::current_path(std::filesystem::path(argv[0]).parent_path());
    sh::Shell *shell{};
    int code{0};

    if (auto res = sh::createShell(&shell); res != sh::Result::Success) {
      err("Failed to create CLI shell: " + sh::to_string(res));
      return 1;
    }

    if (auto res = sh::run(shell); res != sh::Result::Success) {
      err("Shell crashed: " + sh::to_string(res));
      code = 1;
    }

    sh::destroyShell(shell);
    return code;
  }

  catch (std::exception const &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return 1;
  }
}
