#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <sstream> // ostringstream
#include <random>

#include "minisocket.hpp"
#include "dbconn.hpp"
#include "main.h"

#include "json.hpp"
using json = nlohmann::json;

minisocket::Server server;
dbConn::Connector dbConnector("127.0.0.1", "12345");

// having these in global means they're available in all threads
std::string word;
std::set<char> answer;
std::set<char> guesses;
std::set<char> correctGuesses;

std::unordered_map<int, ClientState> clients;

int getRandom(int N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, N);

    return dist(gen);
}

void getWordFromDB() {
    // number of words in the DB
    int N = 2;
    int num = getRandom(N);

    std::ostringstream sqlString;
    sqlString << "SELECT * FROM hangTable WHERE id = " << num << ";";
    json response = dbConnector.connectQueryClose(sqlString.str());
    word = response[0].value("word", "");
    if (word.empty()) throw std::runtime_error("unable to retrieve word from DB");

    answer = std::set<char>(word.begin(), word.end());
}

json serializeClientState(ClientState clientState) {
    json meta{{"username", clientState.meta.username}};
    json gameState{{"isWin", clientState.gameState.isWin}, {"guessState", clientState.gameState.guessState}};
    json state{{"meta", meta}, {"gameState", gameState}};
    return state;
}

void sendGameState(int client_fd, json state) {
    server.sendFrame(client_fd, state.dump(2));
}

void sendGameStateAll() {
    for (auto& [client_fd, clientState] : clients) {
        sendGameState(client_fd, serializeClientState(clientState));
    }
}

bool isCharInSet(char ch, std::set<char> set) {
    return set.find(ch) != set.end();
}

std::string determineGuessState() {

    std::ostringstream guessState;

    for (auto ch : word) {
        if (isCharInSet(ch, correctGuesses)) {
            guessState << ch << " ";
        } else {
            guessState << "_ ";
        }
        guessState << "\n";
    }

    return guessState.str();

}

void resetGame(int client_fd) {
    
    guesses.clear();
    correctGuesses.clear();
    getWordFromDB();

    std::string guessState = determineGuessState();

    for (auto& [client_fd, clientState] : clients) {
        clientState.gameState.isWin = false;
        clients[client_fd].gameState.guessState = guessState;
    }

    sendGameStateAll();
    
}

void handleInitialConnect(int client_fd, json request) {
    ClientState clientState;
    clientState.meta.username = request.value("username", "");
    clientState.gameState.guessState = determineGuessState();
    clients[client_fd] = clientState;
}

void onMessage(int client_fd, const std::string& msg) {

    if (msg.empty()) return;

    json request = json::parse(msg);

    // check if this is the initial connection, if so send current state
    if (request.value("initialConnect", false)) {
        handleInitialConnect(client_fd, request);
        sendGameState(client_fd, serializeClientState(clients[client_fd]));
        return;
    }

    // check if need to reset the game
    if (request.value("isReset", false)) {
        resetGame(client_fd);
        return;
    }

    std::string letter = request.value("letter", "");
    if (letter.empty()) return;

    char ch = letter[0];
    guesses.insert(ch);

    if (isCharInSet(ch, answer)) correctGuesses.insert(ch);

    // clients are sharing game state in this iteration
    std::string guessState = determineGuessState(); 
    bool isWin = correctGuesses.size() == answer.size();
    
    for (auto& [client_fd, clientState] : clients) {
        clientState.gameState.guessState = guessState;
        clientState.gameState.isWin = isWin;
    }

    sendGameStateAll();
}

int main() {

    getWordFromDB();

    server.init("9002", &onMessage, true);
    std::cout << "Server started on " << "9002" << std::endl;

    server.run();
    return 0;
}