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
		server_address.sin_addr.s_addr = gethostid();
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(8476);
	
		if (bind(this->server_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address))) {
			FMS_ERR("failed to bind socket.")
			return false;
		}
	
		listen(this->server_socket, 2);
		s32 myPrio;
		svcCreateEvent(&this->ev_stop, RESET_ONESHOT);
		svcCreateEvent(&this->ev_send, RESET_ONESHOT);
		svcCreateEvent(&this->ev_received, RESET_ONESHOT);
		svcGetThreadPriority(&myPrio, CUR_THREAD_HANDLE);
		this->acceptThread = threadCreate(&REServer::handleAccept, this, 0x1000, myPrio + 1, -2, false);
		char tmp[100];
		snprintf(tmp, 100, "Socket opened on port 8476 on %s", inet_ntoa(server_address.sin_addr));
		FMS_INF(tmp)
		
		return true;
	}
	
	void REServer::close() {
		std::cout << "Signalling to stop" << std::endl;
		svcSignalEvent(this->ev_stop);
		threadJoin(this->acceptThread, U64_MAX);
		threadFree(this->acceptThread);
	}
	
	void REServer::handleAccept(void* args) {
		REServer* self = static_cast<REServer*>(args);
		
        const size_t MAX_PFDS = 1;
		std::vector<struct pollfd> pfds(1);
		std::vector<SocketState> socketStates;
		pfds[0].fd = self->server_socket;
		pfds[0].events = POLLIN;
		
		struct sockaddr_storage otherSock;
		socklen_t otherSockSize = sizeof(otherSock);
		
		while(true) {
			int pollCount = poll(pfds.data(), pfds.size(), 100);
			// Check if we're woken up by the stop event
			Result stopRes = svcWaitSynchronization(self->ev_stop, 0);
			if (stopRes == 0) break;
			
			if (pollCount > 0) {
				//std::cout << "Something happened!" << std::endl;
				
				if (pfds[0].revents & POLLIN) {
                    // We only support 1 socket at the moment.
                    if (pfds.size() - 1 < MAX_PFDS) {
                        // A new socket wants to be accepted
                        int otherSockFD = accept(self->server_socket, reinterpret_cast<struct sockaddr *>(&otherSock), &otherSockSize);
                        std::cout << "Hi!" <<  otherSockFD << std::endl;
                        if (otherSockFD > 0) {
                            struct pollfd newFd;
                            newFd.fd = otherSockFD;
                            newFd.events = POLLIN | POLLOUT;
                            pfds.push_back(newFd);
                            socketStates.push_back(SocketState());
                        }
                    }
				}
				
				int i = 0;
				for (auto it = pfds.begin() + 1; it != pfds.end(); i++) {
					struct pollfd cur = *it;
					if (cur.revents & POLLIN) {
						const size_t R_BUF_SIZE = 256;
						u8 mesg[R_BUF_SIZE];
						ssize_t received = recv(cur.fd, mesg, R_BUF_SIZE, 0);
						if (received == 0) {
							std::cout << "Socket closed" << std::endl;
							::close(cur.fd);
							it = pfds.erase(it);
							socketStates.erase(socketStates.begin() + i);
							i -= 1;
						} else if (received < 0) {
							std::cout << "Error occured: " << errno <<	": " << strerror(errno) << std::endl;
							::close(cur.fd);
							it = pfds.erase(it);
							socketStates.erase(socketStates.begin() + i);
						} else {
							std::cout << "Received " << received << " bytes from socket: " /*<< mesg*/ << std::endl;
							//::send(cur.fd, mesg, received, 0);
							self->updateState(socketStates[i], mesg, received);
							it++;
						}
					} else if(cur.revents & POLLOUT) {
                        if (!self->pendingIPPackets.empty()) {
                            std::vector<u8> packet = self->pendingIPPackets.pop_front();
                            ssize_t sent = send(cur.fd, packet.data(), packet.size(), 0);
                            if (sent < 0) {
                                std::cout << "Error occured: " << errno <<	": " << strerror(errno) << std::endl;
                                ::close(cur.fd);
                                it = pfds.erase(it);
                                socketStates.erase(socketStates.begin() + i);
                            } else if (sent != packet.size()) {
                                std::cout << "Could not send entire packet!" << std::endl;
                                it++;
                            } else {
                                it++;
                            }
                        } else {
                            it++;
                        }
                    } else {
						it++;
					}
				}
			}
		}
		std::cout << "Closing file descriptors" << std::endl;
		// Close socket descriptors
		for (size_t i = 0; i < pfds.size() + 1; i++) {
			::close(pfds[i].fd);
		}
		std::cout << "Stopping listerner thread" << std::endl;
		threadExit(0);
	}
}

void FMS::REServer::updateState(FMS::SocketState &state, u8 *data, size_t newDataLength) {
	state.buffer.reserve(state.buffer.size() + newDataLength);
    std::cout << std::hex;
	for (size_t i = 0; i < newDataLength; i++) {
		state.buffer.push_back(data[i]);
        std::cout << data[i] << " ";
	}
	std::cout << std::dec <<  std::endl;
	
	while (true) {
		std::cout << "Parsing packet of data" << std::endl;
		if (state.expectedPacketSize == 0 && state.buffer.size() >= sizeof(state.expectedPacketSize)) {
			state.expectedPacketSize = static_cast<u32>((state.buffer[0] << 24) | (state.buffer[1] << 16) | (state.buffer[2] << 8) | state.buffer[3]);
			state.buffer.reserve(state.expectedPacketSize + sizeof(state.expectedPacketSize));
            std::cout << "Expecting " << state.expectedPacketSize << " bytes of data" << std::endl;
		} else if (state.expectedPacketSize > 0 && state.expectedPacketSize <= state.buffer.size() - sizeof(state.expectedPacketSize)) {
            std::cout << "Made a packet!" << std::endl;
			// We have received enough data to make a packet!
			std::vector<u8> packet(state.buffer.begin() + 4, state.buffer.begin() + 4 + state.expectedPacketSize);
			pendingIRPackets.push_back(std::move(packet));
			state.buffer.erase(state.buffer.begin(), state.buffer.begin() + 4 + state.expectedPacketSize);
			state.expectedPacketSize = 0;
		} else {
            std::cout << "Waiting for buffer to fill" << std::endl;
			break;
		}
	}
}

