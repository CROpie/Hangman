#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <cstdlib>
#include <sstream> // ostringstream
#include <random>

#include "minisocket.hpp"
#include "dbconn.hpp"
#include "main.h"
#include "databaseService.h"
#include "gameController.h"
#include "hangman.h"

#include "json.hpp"
using json = nlohmann::json;

// void onDisconnect(int client_fd) {
//     std::cout << "Someone has left the game" << std::endl;
//     clients.erase(client_fd);
//     json msg{{"id of player who left", client_fd}};
//     for (auto& [client_fd, clientState] : clients) {
//         server.sendFrame(client_fd, msg.dump(2));
//     }
// }

int main() {

    // library to connect server to DB
    dbConn::Connector dbConnector("127.0.0.1", "12345");

    // helper functions for DB
    DatabaseService dbService(dbConnector);

    // word for first game
    std::string word = dbService.getWordFromDB();

    // initialize first game
    HangmanGame game(word);

    // library to connect server to client
    minisocket::MiniSocket miniSocket("9002");
    
    GameController gameController(miniSocket, game, dbService);

    gameController.start();

    return 0;
}