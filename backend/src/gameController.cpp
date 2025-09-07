#include "gameController.h"

GameController::GameController(minisocket::MiniSocket& miniSocket, HangmanGame& hangmanGame, DatabaseService& dbService)
    : miniSocket(miniSocket), hangmanGame(hangmanGame), dbService(dbService)
    {
        miniSocket.on_message = [&](int client_fd, const std::string& msg) {
            this->onMessage(client_fd, msg);
        };
    }

json GameController::serializePlayerMetadata(Player player) {

    json meta{{"username", player.username}};
    return meta;
}

json GameController::serializeGameState() {
    json gameState{{"isWin", hangmanGame.isWin}, {"guessState", hangmanGame.determineGuessState()}};
    return gameState;
}

void GameController::sendGameStateAll() {

    json gameState = serializeGameState();

    for (auto& player : players) {

        json meta = serializePlayerMetadata(player);
        json state{{"meta", meta}, {"gameState", gameState}};

        miniSocket.sendFrame(player.client_fd, state.dump(2));
    }
}

void GameController::handleInitialConnect(int client_fd, json request) {
    Player newPlayer(client_fd, request["username"]);
    players.emplace_back(newPlayer);
}

void GameController::start() {
    miniSocket.init();

    std::cout << "Server started on " << "9002" << std::endl;

    miniSocket.run();
}

/*
    {
        action: "join" | "play" | "reset"
    }
*/
void GameController::onMessage(int client_fd, const std::string& msg) {
    if (msg.empty()) return;

    json request = json::parse(msg);
    std::string action = request["action"];

    // add player to game if they have just joined
    if (action == "join") {
        handleInitialConnect(client_fd, request);
    }

    // determine new game state
    if (action == "play") {
        std::string letter = request.value("letter", "");
        if (letter.empty()) return;
        hangmanGame.handlePlay(letter[0]);
    }

    // reset game state
    if (action == "reset") {
        std::string newWord = dbService.getWordFromDB();
        hangmanGame.startNewGame(newWord);
    }

    // send game state to all players
    sendGameStateAll();
}