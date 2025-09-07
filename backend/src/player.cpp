#include "player.h"

Player::Player(int client_fd, std::string username)
    : client_fd(client_fd), username(username) {}