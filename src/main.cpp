#include <wtts/interactiveCLI.hpp>

int main(int argc, char **argv) {
  try {
    es::Shell shell{std::cin, std::cout};
    shell.run();
  }

  catch (std::exception const &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return 1;
  }
}
