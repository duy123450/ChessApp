#pragma once

#include "Move.h"
#include "GameState.h"
#include <string>

using namespace std;

string getMoveNotation(
    char piece,
    char target,
    const Move& move,
    const GameState& state,
    bool isEnPassant
);