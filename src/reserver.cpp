#include "reserver.h"

namespace FMS {
	REServer::REServer() {
	
	}

	bool REServer::open() {
		struct sockaddr_in server_address;
		this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (this->server_socket < 0) {
			FMS_ERR("ERROR opening socket")
			return false;
		}
		server_address.sin_addr.s_addr = INADDR_ANY;
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(8476);
	
		if (bind(this->server_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address))) {
			FMS_ERR("failed to bind socket.")
			return false;
		}
	
		listen(this->server_socket, 2);
		this->acceptThread = threadCreate(&REServer::handleAccept, this, 0x200, 0x31, -2, false);
		return true;
	}
	
	void REServer::handleAccept(void* args) {
		REServer* self = reinterpret_cast<REServer*>(args);
		FMS_DBG("Starting to accept sockets")
	}
}
