#pragma once

#include <string>

class Player {
    public:
        int client_fd;
        std::string username;


        Player(int client_fd, std::string username);

    private:

};