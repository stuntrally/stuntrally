#include <iostream>
#include <string>
#include <map>
#include "../enet-wrapper.hpp"
#include "../network.hpp"
#include "../protocol.hpp"


int main(int argc, char** argv) {
	std::string host = "localhost";
	int port = protocol::DEFAULT_PORT;

	// Command line handling
	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg == "--help" || arg == "-h") {
			std::cout << std::endl << "Usage: " << argv[0] << " [PARAMETERS]" << std::endl
				<< std::endl
				<< "Available parameters:" << std::endl
				<< "  -h, --help                  this help" << std::endl
				<< "  -a, --address <hostaddress> master server address to connect to" << std::endl
				<< "                              default: localhost" << std::endl
				<< "  -p, --port <portnumber>     master server port to connect to" << std::endl
				<< "                              default: " << protocol::DEFAULT_PORT << std::endl
				;
			return EXIT_SUCCESS;
		} else if ((arg == "--address" || arg == "-a") && i < argc-1) {
			host = argv[i+1];
			++i;
		} else if ((arg == "--port" || arg == "-p") && i < argc-1) {
			port = atoi(argv[i+1]);
			++i;
		} else {
			std::cout << "Invalid argument " << arg << std::endl;
			return EXIT_FAILURE;
		}
	}

	MasterClient mc;
	std::cout << "Connecting..." << std::endl;
	mc.connect(host, port);
	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

	std::cout << "Creating a test game" << std::endl;
	mc.updateGame("Test game", "M-001", 1);
	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

	std::cout << "Request new list" << std::endl;
	mc.refreshList();

	while (true) {

	}

	return EXIT_SUCCESS;
}
