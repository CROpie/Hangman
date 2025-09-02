#pragma once

#include <string>

struct Meta {
    std::string username{""};
};

struct GameState {
    bool isWin{false};
    std::string guessState{""};
};

struct ClientState {
    Meta meta;
    GameState gameState;
    bool isWaiting{false};
};