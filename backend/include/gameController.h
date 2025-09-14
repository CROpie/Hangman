#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <deque>

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
        struct Chat {
            std::string username;
            std::string chat;
        };

        minisocket::MiniSocket& miniSocket;
        HangmanGame& hangmanGame;
        DatabaseService& dbService;

        std::vector<Player> players;
        std::deque<Chat> chatHistory;

        const size_t MAX_MESSAGES = 10;

        GameController(minisocket::MiniSocket& miniSocket, HangmanGame& hangmanGame, DatabaseService& dbService);

        void start();

    private:
        // receiving data from client
        void onMessage(int client_fd, const std::string& msg);
        void handleInitialConnect(int client_fd, json request);
        void onDisconnect(int client_fd);
        void handlePlayerChat(json request);

        void handleWin();
        void handleLose();

        // sending data back to client
        json serializeGameState();
        json serializePlayerMetadata(Player player);
        json serializeChatHistory();

        void sendGameStateAll();
        void addMessageToArray(std::string message, std::string sender);
        void sendChatHistory();


        // helpers
        Player* findPlayer(int client_fd);


};