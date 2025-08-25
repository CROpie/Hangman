#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <sstream> // ostringstream

#include "minisocket.hpp"

#include "json.hpp"
using json = nlohmann::json;

minisocket::Server server;
std::string word = "peanut";
std::set<char> answer(word.begin(), word.end());
std::set<char> guesses;
std::set<char> correctGuesses;



void sendState(int client_fd, json state) {
    server.sendFrame(client_fd, state.dump(2));
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
    response["guessState"] = determineGuessState();
    sendState(client_fd, response);
}

void onMessage(int client_fd, const std::string& msg) {

    if (msg.empty()) return;

    json request = json::parse(msg);

    // default to false if missing
    bool isReset = request.value("isReset", false);

    if (isReset) {
        resetGame(client_fd);
        return;
    }

    std::string letter = request.value("letter", "");
    if (letter.empty()) return;

    char ch = letter[0];
    guesses.insert(ch);

    json response = {{"isWin", false}};

    if (isCharInSet(ch, answer)) correctGuesses.insert(ch);

    response["guessState"] = determineGuessState();

    if (correctGuesses.size() == answer.size()) response["isWin"] = true;

    sendState(client_fd, response);
}

int main() {

    const char* port = "9002";

    server.init(port, &onMessage, true);
    std::cout << "Server started on " << port << std::endl;

    server.run();
    return 0;
}