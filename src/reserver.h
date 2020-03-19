#ifndef RESERVER_H
#define RESERVER_H

#include <3ds.h>

#include <iostream>
#include <optional>
#include <queue>
#include <vector>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <netinet/in.h>

#include "debug.h"

namespace FMS {
    
    /**
     * Reverse-Engineer Server. Used to send received IR data to a connected computer,
     * and to send received packets back over IR.
     */
    class REServer {
    public:
        REServer();
        /**
         * Opens the socket and starts listening
         * \returns True if opening was successfull.
         */
        bool open();
        void sendPacket(std::vector<u8> packet) {}
        std::optional<std::vector<u8>> receivePacket();
    private:
        /**
         * Gets called when a new connection has been made.
         */
        void handleClient(void* args);
		static void handleAccept(void* args);
		
		Thread acceptThread;
		
        struct handleClientArgs {
            int client_socket;
        };
        int server_socket;
        bool hasConnection = false;
		bool requestedStop = false;
        /**
         * Packets still to be sent to the computer.
         */
        std::queue<std::vector<u8>> pendingIPPackets;
        /**
         * Packets still needed to be sent over IR.
         */
        std::queue<std::vector<u8>> pendingIRPackets;
    };
    
}
#endif //RESERVER_H
