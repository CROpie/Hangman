#pragma once

#include <iostream>
#include <string>
#include <set>
#include <cstdlib>
#include <sstream> // ostringstream
#include <random>
#include "json.hpp"
using json = nlohmann::json;

class HangmanGame {
    public:
        int MISSES_TO_LOSE{12};

        std::string word;
        std::set<char> answer;
        std::set<char> guesses;
        std::set<char> correctGuesses;
        bool isWin{false};
        bool isLose{false};
        int misses{0};

        HangmanGame(const std::string& word);
        
        // game logic
        bool isCharInSet(char ch, std::set<char> set);
        std::string determineGuessState();

        // dealing with user input
        void handlePlay(char letter);
        void startNewGame(std::string newWord);

    private:

};