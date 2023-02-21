#include <core/types.h>
#include <platform/iradapter.h>
#include <platform/iradaptermanager.h>
#include <protocol/link.h>

#include <iomanip>
#include <iostream>

using namespace fms;
using namespace fms::platform;

int main(int argc, char** argv) {
	IRAdapterManager& adapterMgr = IRAdapterManager::getInstance();
	IRAdapter adapter = adapterMgr.list()[0];
	fms::protocol::LinkConnection con(adapter);
	con.accept();

	std::vector<u8> data(25);
	//size_t sent = 0;
	//adapter.read(data.begin(), data.end(), sent);
	auto sent = con.receive(data.begin(), data.end());
	std::cout << "Read " << sent << " bytes" << std::endl;
	int i = 1;
	for (auto it = data.begin(); it != data.begin() + sent; it++) {
		std::cout << std::hex <<  std::setw(2) << std::setfill('0') << unsigned(*it);
		if (i % 4 == 0) std::cout << " ";
		if (i % 16 == 0) std::cout << std::endl;
		i++;
	}
	std::cout << std::endl;
	return 0;
}
