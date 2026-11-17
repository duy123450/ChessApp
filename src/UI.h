
#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>

#include "GameData.h"

class UI
{
private:
    // Font reference
    sf::Font &font;

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

    sf::RectangleShape claimDrawButton;
    sf::Text claimDrawButtonText;

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

    // Pawn promotion popup
    sf::RectangleShape promotionOverlay;
    sf::RectangleShape promotionPanel;

    sf::Text promotionTitle;
    sf::Text majorLabel;
    sf::Text minorLabel;

    // Promotion buttons
    sf::RectangleShape queenButton;
    sf::RectangleShape rookButton;
    sf::RectangleShape bishopButton;
    sf::RectangleShape knightButton;

    sf::Text queenText;
    sf::Text rookText;
    sf::Text bishopText;
    sf::Text knightText;

    // Game-over popup
    sf::RectangleShape gameOverOverlay;
    sf::RectangleShape gameOverPanel;

    sf::Text gameOverText;
    sf::Text gameOverSubtitle;

public:
    UI(sf::Font &font);

    void drawPanelsAndHistory(
        sf::RenderWindow &window,
        const GameData &game,
        int visibleRows);

    void drawPlayerInfo(
        sf::RenderWindow &window,
        const GameData &game);

    void drawLoadConfirmation(
        sf::RenderWindow &window,
        bool showLoadConfirmation);

    void drawPromotionPopup(
        sf::RenderWindow& window);

    char handlePromotionClick(
        float mouseX,
        float mouseY);
    void drawClaimDrawButton(
        sf::RenderWindow& window);

    bool isClaimDrawButtonClicked(
        float mouseX,
        float mouseY);

    void drawGameOverPopup(
        sf::RenderWindow& window,
        const GameData& game);
};

#endif
