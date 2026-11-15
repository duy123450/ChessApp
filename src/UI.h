
#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>

#include "GameData.h"

class UI {
private:
    // Font reference
    sf::Font& font;

    // Player information
    sf::Text blackPlayerText;
    sf::Text whitePlayerText;
    sf::Text blackAdvantageText;
    sf::Text whiteAdvantageText;

    // Panels
    sf::RectangleShape blackPanel;
    sf::RectangleShape whitePanel;
    sf::RectangleShape moveHistoryPanel;

    // Buttons
    sf::RectangleShape newGameButton;
    sf::Text newGameButtonText;

    sf::RectangleShape saveGameButton;
    sf::Text saveGameButtonText;

    sf::RectangleShape loadGameButton;
    sf::Text loadGameButtonText;

    // Move history
    sf::Text moveHistoryTitle;
    sf::Text moveNumberHeader;
    sf::Text whiteMoveHeader;
    sf::Text blackMoveHeader;

    // Load confirmation dialog
    sf::RectangleShape confirmationOverlay;
    sf::RectangleShape confirmationPanel;

    sf::Text confirmationTitle;
    sf::Text confirmationMessage;

    sf::RectangleShape confirmLoadButton;
    sf::Text confirmLoadText;

    sf::RectangleShape cancelLoadButton;
    sf::Text cancelLoadText;

public:
    UI(sf::Font& font);

    void drawPanelsAndHistory(
        sf::RenderWindow& window,
        const GameData& game,
        int visibleRows
    );

    void drawPlayerInfo(
        sf::RenderWindow& window,
        const GameData& game
    );

    void drawLoadConfirmation(
        sf::RenderWindow& window,
        bool showLoadConfirmation
    );
};

#endif
