#include <wtts/dataParser.hpp>
#include <iostream>

int main() {
	dp::DataParser* p = new dp::XMLDataParser{};
	p->loadData("/mnt/c/Users/User/Desktop/dataStorage.xml");
	auto ids = p->getUserIdentifiers();

	for (auto const& id : ids) {
		std::cout << p->getUserName(id) << " " << p->getUserSurname(id) << std::endl;
	}

	delete p;
}
