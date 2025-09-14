#include "gameController.h"

GameController::GameController(minisocket::MiniSocket& miniSocket, HangmanGame& hangmanGame, DatabaseService& dbService)
    : miniSocket(miniSocket), hangmanGame(hangmanGame), dbService(dbService)
    {
        miniSocket.on_message = [&](int client_fd, const std::string& msg) {
            this->onMessage(client_fd, msg);
        };

        miniSocket.on_disconnect = [&](int client_fd) {
            this->onDisconnect(client_fd);
        };
    }

json GameController::serializePlayerMetadata(Player player) {

    json meta{{"username", player.username}};
    return meta;
}

json GameController::serializeGameState() {
    json gameState{
        {"isWin", hangmanGame.isWin},
        {"isLose", hangmanGame.isLose},
        {"guessState", hangmanGame.determineGuessState()},
        {"misses", hangmanGame.misses}
        };
    return gameState;
}

json GameController::serializeChatHistory() {
    json jsonChatHistory = json::array();
    for (auto& chat : chatHistory) {
        jsonChatHistory.push_back({
            { "sender", chat.username },
            { "message", chat.chat },
        });
    }
    return jsonChatHistory;
}

void GameController::sendChatHistory() {
    std::cout << "Sending Chat History" << std::endl;

    json jsonChatHistory = serializeChatHistory();

    std::cout << "chat history: " << jsonChatHistory.dump(2) << std::endl;


    json message{{"type", "message"}, {"chatHistory", jsonChatHistory}};
    for (auto& player : players) {
        miniSocket.sendFrame(player.client_fd, message.dump(2));
    }
}

void GameController::sendGameStateAll() {

    json gameState = serializeGameState();

    for (auto& player : players) {

        json meta = serializePlayerMetadata(player);
        json state{{"type", "gameState"}, {"meta", meta}, {"gameState", gameState}};

        miniSocket.sendFrame(player.client_fd, state.dump(2));
    }
}

void GameController::handleInitialConnect(int client_fd, json request) {
    Player newPlayer(client_fd, request["username"]);
    players.emplace_back(newPlayer);

    std::string message = newPlayer.username + " has joined the game.";

    addMessageToArray(message, "system");
    sendChatHistory();
}

void GameController::handlePlayerChat(json request) {
    std::string chat = request.value("chat", "");
    std::string username = request.value("username", "");
    addMessageToArray(chat, username);
    sendChatHistory();
}

void GameController::addMessageToArray(std::string message, std::string sender) {
    Chat newChat{sender, message};

    if (chatHistory.size() == MAX_MESSAGES) chatHistory.pop_front();

    chatHistory.push_back(newChat);
}

void GameController::start() {
    miniSocket.init();

    std::cout << "Server started on " << "9002" << std::endl;

    miniSocket.run();
}

void GameController::handleWin() {
    addMessageToArray("You win!", "system");
    sendChatHistory();
}

void GameController::handleLose() {
    addMessageToArray("You LOSE!", "system");
    sendChatHistory();
}

/*
    {
        action: "join" | "play" | "reset" | "message"
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
        
        if (hangmanGame.isWin) handleWin();
        if (hangmanGame.isLose) handleLose();
    }

    // reset game state
    if (action == "reset") {
        std::string newWord = dbService.getWordFromDB();
        hangmanGame.startNewGame(newWord);
    }

    if (action == "chat") {
        handlePlayerChat(request);
    }

    // send game state to all players
    sendGameStateAll();
}

Player* GameController::findPlayer(int client_fd) {
    auto it = std::find_if(players.begin(), players.end(),
        [client_fd](const Player& p) {
            return p.client_fd == client_fd;
        });

    return (it != players.end()) ? &(*it) : nullptr;
}

void GameController::onDisconnect(int client_fd) {

    Player* playerToRemove = findPlayer(client_fd);
    std::string message = playerToRemove->username + " has left the game.";

    // remove from gameController
    players.erase(
        std::remove_if(players.begin(), players.end(),
            [client_fd](const Player& p) { return p.client_fd == client_fd; }),
        players.end()
    );

    // remove from miniSocket
    miniSocket.remove_client(client_fd);

    if (players.size() > 0) {
        addMessageToArray(message, "system");
        sendChatHistory();
        return;
    }

    // clear all values if no players remain
    chatHistory.clear();
    std::string newWord = dbService.getWordFromDB();
    hangmanGame.startNewGame(newWord);

}