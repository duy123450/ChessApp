
#ifndef GAME_H
#define GAME_H

#include "GameData.h"

class Game {
private:
    GameData& data;

public:
    Game(GameData& gameData);

    void reset();

    void handleBoardClick(int row, int col);

    void handlePromotion(char promotionChoice);

    void handleDrawClaim(char key);
};

#endif
