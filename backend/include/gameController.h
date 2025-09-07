#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <cstdlib>
#include <sstream> // ostringstream
#include <random>

#include "json.hpp"

#include "minisocket.hpp"

#include "databaseService.h"

#include "hangman.h"
#include "player.h"


using json = nlohmann::json;

class GameController {
    public:
        minisocket::MiniSocket& miniSocket;
        HangmanGame& hangmanGame;
        DatabaseService& dbService;

        std::vector<Player> players;

        GameController(minisocket::MiniSocket& miniSocket, HangmanGame& hangmanGame, DatabaseService& dbService);

        void start();

    private:
        // receiving data from client
        void onMessage(int client_fd, const std::string& msg);
        void handleInitialConnect(int client_fd, json request);

        // sending data back to client
        json serializeGameState();
        json serializePlayerMetadata(Player player);
        void sendGameStateAll();

};