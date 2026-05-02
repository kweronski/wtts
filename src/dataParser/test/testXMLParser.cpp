#include "wtts/dataParser.hpp"
#include <iostream>
#include <wtts/xmlParser.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " <path to xml data> <path to txt cross reference>\n";
    return 1;
  }

  dp::XMLDataParser parser{argv[1]};
  if (auto result = parser.loadData(); result != dp::Result::Success) {
    // something went wrong
    return 1;
  }

  /* LOGIC:
   * Use DataParser interface to extract information from XML file.
   * Use txt file to verify that the extracted data from the XML file
   * is correct.
   */
  return 0;
}
