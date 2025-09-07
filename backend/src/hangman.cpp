#include "hangman.h"


HangmanGame::HangmanGame(const std::string& word)
    : word(word)
    {
        answer = std::set<char>(word.begin(), word.end());
    }


bool HangmanGame::isCharInSet(char ch, std::set<char> set) {
    return set.find(ch) != set.end();
}

std::string HangmanGame::determineGuessState() {

    std::ostringstream guessState;

    for (auto ch : HangmanGame::word) {
        if (isCharInSet(ch, HangmanGame::correctGuesses)) {
            guessState << ch << " ";
        } else {
            guessState << "_ ";
        }
        guessState << "\n";
    }

    return guessState.str();

}

void HangmanGame::handlePlay(char letter) {

    guesses.insert(letter);

    if (isCharInSet(letter, answer)) correctGuesses.insert(letter);

    std::string guessState = determineGuessState(); 

    isWin = correctGuesses.size() == answer.size();
}

void HangmanGame::startNewGame(std::string newWord) {
    guesses.clear();
    correctGuesses.clear();
    isWin = false;

    word = newWord;
    answer = std::set<char>(word.begin(), word.end());
    std::string guessState = determineGuessState(); 
}