#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include "GameData.h"

class SaveManager {
public:
    static void clearSavedGame();
    static void saveGame(const GameData& game);
    static bool loadGame(GameData& game);
};

#endif
