#include <wtts/dataParser.hpp>
#include <pugixml.hpp>

namespace dp {
		void destroyDocument(pugi::xml_document* p) { delete p; }
		XMLDataParser::XMLDataParser() : document_{new pugi::xml_document{}, destroyDocument} {
		}

		Result XMLDataParser::loadData(std::string const& url) {
			document_->load_file(url.c_str());
			return Result::Success;
		}
		std::vector<XMLDataParser::ID> XMLDataParser::getUserIdentifiers() {
			std::vector<XMLDataParser::ID> ids;
			return ids;
		}

		// Personal info
		bool XMLDataParser::getUserActiveStatus(ID const& id) {
			return true;
		}
		std::string XMLDataParser::getUserName(ID const& id) {
			return "";
		}
		std::string XMLDataParser::getUserSurname(ID const& id) {
			return "";
		}
		std::string XMLDataParser::getUserTelephone(ID const& id) {
			return "";
		}

		// Employee info
		unsigned XMLDataParser::getUserStandardWorkTime(ID const& id) {
			return 0;
		}
		unsigned XMLDataParser::getUserMaxWorkTime(ID const& id) {
			return 0;
		}
		unsigned XMLDataParser::getUserHourlyWage(ID const& id) {
			return 0;
		}
		unsigned XMLDataParser::getUserRole(ID const& id) {
			return 0;
		}
		std::string XMLDataParser::getUserCardId(ID const& id) {
			return "";
		}
}
