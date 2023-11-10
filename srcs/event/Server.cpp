#define SERVER_LEVEL (1 << 2)
#define LOCATION_LEVEL (1 << 3)

#include "Server.hpp"
#include "Connection.hpp"
#include "WebServer.hpp"

Server::Server(int port) : mPort(port) {
  mSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (mSocket < 0) {
    // FD limit exceededw
    throw std::runtime_error("Error: socket() creation failed: " +
                             std::string(strerror(errno)));
  }

  fcntl(mSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

  memset(&mAddr, 0, sizeof(mAddr));
  mAddr.sin_family = AF_INET;
  mAddr.sin_port = htons(mPort);
  mAddr.sin_addr.s_addr = htonl(0);

  int optval = 1;
  if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) <
      0) {
    throw std::runtime_error("Error: Failed to set socket option: " +
                             std::string(strerror(errno)));
  }

  if (bind(mSocket, (struct sockaddr *)&mAddr, sizeof(mAddr)) < 0) {
    throw std::runtime_error("Error: Failed to bind the socket: " +
                             std::string(strerror(errno)));
  }

  if (listen(mSocket, SOMAXCONN /* backlog size*/) < 0) {
    throw std::runtime_error("Error: Failed to listen on the socket: " +
                             std::string(strerror(errno)));
  }

  struct kevent event;
  EV_SET(&event, mSocket, EVFILT_READ, EV_ADD, 0, 0, this);
  kevent(Common::mKqueue, &event, 1, NULL, 0, NULL);
}

Server::~Server() {
  // Remove the socket from the mKqueue
  struct kevent event;
  EV_SET(&event, mSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
  kevent(Common::mKqueue, &event, 1, NULL, 0, NULL);

  // Close the socket
  mConnection.clear();
  close(mSocket);
}

void Server::EventHandler(struct kevent &currentEvent) {
  if (currentEvent.flags & EV_ERROR) {
    return;
  }
  switch (currentEvent.filter) {
  case EVFILT_READ:
    readHandler();
    break;
  case EVFILT_TIMER:
    timerHandler();
    break;
  default:
    break;
  }
}

void Server::readHandler() {
  int socket = accept(mSocket, NULL, NULL);
  if (socket == -1) {
    std::cout << RED << "Error: Failed to accept the connection: "
              << std::string(strerror(errno)) << RESET << std::endl;
  }

  mConnection[socket] = SharedPtr<Connection>(new Connection(socket, mPort));
}


void Server::timerHandler() {}

