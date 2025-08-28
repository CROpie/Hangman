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

#include "json.hpp"
using json = nlohmann::json;

minisocket::Server server;
dbConn::Connector dbConnector("127.0.0.1", "12345");

std::string word;
std::set<char> answer;
std::set<char> guesses;
std::set<char> correctGuesses;

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

void sendState(int client_fd, json state) {
    // server.sendFrame(client_fd, state.dump(2));
    server.sendFrameToAll(state.dump(2));
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
    json response = {{"isWin", false}};
    guesses.clear();
    correctGuesses.clear();
    getWordFromDB();
    response["guessState"] = determineGuessState();
    sendState(client_fd, response);
}

void onMessage(int client_fd, const std::string& msg) {

    if (msg.empty()) return;

    json request = json::parse(msg);

    json response = {{"isWin", false}};

    // check if this is the initial connection, if so send current state
    if (request.value("initialConnect", false)) {
        response["guessState"] = determineGuessState();
        sendState(client_fd, response);
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

    response["guessState"] = determineGuessState();

    if (correctGuesses.size() == answer.size()) response["isWin"] = true;

    sendState(client_fd, response);
}

int main() {

    getWordFromDB();

    server.init("9002", &onMessage, true);
    std::cout << "Server started on " << "9002" << std::endl;

    server.run();
    return 0;
}