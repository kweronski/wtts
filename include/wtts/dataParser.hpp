#include <memory>
#include <vector>
#include <string>

namespace pugi {
	class xml_document;
}

namespace dp {
enum class Result {
	Success,
	CouldNotOpenFileError,
	MissingUserActiveStatusError,
	MissingUserName,
	MissingUserSurname,
	MissingUserTelephone,
	MissingUserStandardWorkTime,
	MissingUserMaxWorkTime,
	MissingUserHourlyWage,
	MissingUserRole,
	MissingUserCardId,
	MissingUserId
};

class DataParser {
	public:
		virtual Result loadData(std::string const& url) = 0;
		using ID = std::string;
		virtual std::vector<ID> getUserIdentifiers() = 0;

		// Personal info
		virtual bool getUserActiveStatus(ID const& id) = 0;
		virtual std::string getUserName(ID const& id) = 0;
		virtual std::string getUserSurname(ID const& id) = 0;
		virtual std::string getUserTelephone(ID const& id) = 0;

		// Employee info
		virtual unsigned getUserStandardWorkTime(ID const& id) = 0;
		virtual unsigned getUserMaxWorkTime(ID const& id) = 0;
		virtual unsigned getUserHourlyWage(ID const& id) = 0;
		virtual unsigned getUserRole(ID const& id) = 0;
		virtual std::string getUserCardId(ID const& id) = 0;

		// Attendance info
		virtual ~DataParser() = default;
};

class XMLDataParser : public DataParser {
	public:
		XMLDataParser();
		Result loadData(std::string const& filePath) override;
		std::vector<ID> getUserIdentifiers() override;

		// Personal info
		bool getUserActiveStatus(ID const& id) override;
		std::string getUserName(ID const& id) override;
		std::string getUserSurname(ID const& id) override;
		std::string getUserTelephone(ID const& id) override;

		// Employee info
		unsigned getUserStandardWorkTime(ID const& id) override;
		unsigned getUserMaxWorkTime(ID const& id) override;
		virtual unsigned getUserHourlyWage(ID const& id) override;
		virtual unsigned getUserRole(ID const& id) override;
		virtual std::string getUserCardId(ID const& id) override;

	private:
		std::unique_ptr<pugi::xml_document, void(*)(pugi::xml_document*)> document_;
};
}
