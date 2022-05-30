#include "reserver.h"

namespace fms {
    
    std::optional<Packet*> Packet::fromVector(const std::vector<u8> &data) {
        if (data.size() < PACKET_HEADER_SIZE) {
            return std::nullopt;
        }
        PacketType type = static_cast<PacketType>((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
        if (type & PacketType::REPLY_FLAG && data.size() >= PACKET_HEADER_SIZE + sizeof(Result)) {
			Result res = static_cast<Result>(data[8] | (data[9] << 8) | (data[10] << 16) | data[11] << 24);
			
			std::optional<std::vector<u8>> payload = std::nullopt;
			if (data.size() > PACKET_HEADER_SIZE + sizeof(Result)) {
				payload = std::vector<u8>(data.begin() + PACKET_HEADER_SIZE + sizeof(Result), data.end());
			}
			
            return new ReplyPacket(type, res, payload);
        } else {
			std::optional<std::vector<u8>> payload = std::nullopt;
			if (data.size() > PACKET_HEADER_SIZE) {
				payload = std::vector<u8>(data.begin() + PACKET_HEADER_SIZE, data.end());
			}
			return new Packet(type, payload);
		}
    }
    
	std::vector<u8> Packet::asVector() {
		u32 payloadSize = (this->m_payload.has_value() ? this->m_payload.value().size() : 0);
		std::vector<u8> data(PACKET_HEADER_SIZE + payloadSize);
		data[0] =  this->m_type        & 0xff;
		data[1] = (this->m_type >> 8 ) & 0xff;
		data[2] = (this->m_type >> 16) & 0xff;
		data[3] = (this->m_type >> 24) & 0xff;
		data[4] =  payloadSize        & 0xff;
		data[5] = (payloadSize >> 8 ) & 0xff;
		data[6] = (payloadSize >> 16) & 0xff;
		data[7] = (payloadSize >> 24) & 0xff;
		if (this->m_payload.has_value()) {
			auto data_it = data.begin() + PACKET_HEADER_SIZE;
			for(auto it = this->m_payload.value().begin(); it < this->m_payload.value().end(); it++, data_it++) {
				*data_it = *it;
			}
		}
		return data;
	}
	
	std::vector<u8> ReplyPacket::asVector() {
		u32 payloadSize = (this->m_payload.has_value() ? this->m_payload.value().size() : 0  + sizeof(Result));
		std::vector<u8> data(PACKET_HEADER_SIZE + payloadSize);
		data[0] =  this->m_type        & 0xff;
		data[1] = (this->m_type >> 8 ) & 0xff;
		data[2] = (this->m_type >> 16) & 0xff;
		data[3] = (this->m_type >> 24) & 0xff;
		data[4] =  payloadSize        & 0xff;
		data[5] = (payloadSize >> 8 ) & 0xff;
		data[6] = (payloadSize >> 16) & 0xff;
		data[7] = (payloadSize >> 24) & 0xff;
		data[8]  =  this->m_result        & 0xff;
		data[9]  = (this->m_result >> 8 ) & 0xff;
		data[10] = (this->m_result >> 16) & 0xff;
		data[11] = (this->m_result >> 24) & 0xff;
		
		if (this->m_payload.has_value()) {
			auto data_it = data.begin() + PACKET_HEADER_SIZE + sizeof(Result);
			for(auto it = this->m_payload.value().begin(); it < this->m_payload.value().end(); it++, data_it++) {
				*data_it = *it;
			}
		}
		return data;
	}
    
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
		svcCreateEvent(&this->ev_stop, RESET_STICKY);
		svcCreateEvent(&this->ev_send, RESET_ONESHOT);
		svcCreateEvent(&this->ev_received, RESET_ONESHOT);
		svcGetThreadPriority(&myPrio, CUR_THREAD_HANDLE);
		this->acceptThread = threadCreate(&REServer::handleAccept, this, 0x1000, myPrio + 1, -2, false);
		this->irComsThread = threadCreate(&REServer::handleIrComms, this, 0x1000, myPrio + 1, -2, false);
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
		// To make the thread stop
		this->pendingIRPackets.push_back(new Packet(NONE));
		threadJoin(this->irComsThread, U64_MAX);
		threadFree(this->irComsThread);
		svcClearEvent(this->ev_stop);
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
                            Packet* p = self->pendingIPPackets.pop_front();
							std::vector<u8> packet = p->asVector();
							delete p;
							
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

void fms::REServer::handleIrComms(void* args) {
	REServer* self = static_cast<REServer*>(args);
	const u64 TIMEOUT = 100000000;
	const u32 RECV_BUF_SIZE = 2000;
	
	while (true) {
		// Check if we're woken up by the stop event
		Result stopRes = svcWaitSynchronization(self->ev_stop, 0);
		if (stopRes == 0) break;
		Packet* p = self->pendingIRPackets.pop_front();
		Result res;
		switch (p->type()) {
			case RECEIVE_PACKET:
				if (p->payload().has_value()) {
					u32 length;
					std::vector<u8> received = std::vector<u8>(RECV_BUF_SIZE);
					res = fms::Link::blockReceivePacket(received.data(), received.size(), &length, TIMEOUT);
					received.erase(received.begin() + length, received.end());
					self->pendingIPPackets.push_back(new ReplyPacket(RECEIVE_PACKET_REPLY, res, received));
				} else {
					std::cout << "Invalid RECEIVE packet" << std::endl;
				}
				break;
			case SEND_PACKET:
				if (p->payload().has_value()) {
					res = fms::Link::blockSendPacket(p->payload().value().data(), p->payload().value().size(), false);
					self->pendingIPPackets.push_back(new ReplyPacket(RECEIVE_PACKET_REPLY, res));
				} else {
					std::cout << "Invalid SEND packet" << std::endl;
				}
				break;
			default:
				std::cout << "Unknown packet type: " << std::hex << p->type() << std::dec << std::endl;
		}
		delete p;
	}
}


void fms::REServer::updateState(fms::SocketState &state, u8 *data, size_t newDataLength) {
	state.buffer.reserve(state.buffer.size() + newDataLength);
    std::cout << std::hex;
	for (size_t i = 0; i < newDataLength; i++) {
		state.buffer.push_back(data[i]);
        std::cout << data[i] << " ";
	}
	std::cout << std::dec <<  std::endl;
	
	while (true) {
		std::cout << "Parsing packet of data" << std::endl;
		if (state.nextType == NONE && state.buffer.size() >= PACKET_HEADER_SIZE) {
			state.nextType = static_cast<PacketType>((state.buffer[0] << 24) | (state.buffer[1] << 16) | (state.buffer[2] << 8) | state.buffer[3]);
			state.expectedPacketSize = static_cast<u32>((state.buffer[4] << 24) | (state.buffer[5] << 16) | (state.buffer[6] << 8) | state.buffer[7]);
			state.buffer.reserve(state.expectedPacketSize + sizeof(state.expectedPacketSize));
            std::cout << "Expecting " << state.expectedPacketSize << " bytes of data" << std::endl;
		} else if (state.nextType != NONE && state.expectedPacketSize <= state.buffer.size() - PACKET_HEADER_SIZE) {
			// We have received enough data to make a packet!
			std::optional<Packet*> packet = Packet::fromVector(std::vector<u8>(state.buffer.begin(), state.buffer.begin() + PACKET_HEADER_SIZE + state.expectedPacketSize));
			if (packet.has_value()) {
				pendingIRPackets.push_back(packet.value());
				std::cout << "Made a packet!" << std::endl;
			} else {
				std::cout << "Encountered malformed packet" << std::endl;
			}
			state.buffer.erase(state.buffer.begin(), state.buffer.begin() + PACKET_HEADER_SIZE + state.expectedPacketSize);
			state.expectedPacketSize = 0;
			state.nextType = NONE;
		} else {
            std::cout << "Waiting for buffer to fill" << std::endl;
			break;
		}
	}
}

