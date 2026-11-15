
#include "UI.h"

#include <algorithm>
#include <string>

using namespace sf;
using namespace std;

UI::UI(Font& font)
    : font(font),
    blackPlayerText(font, "Player 2 - Black", 24),
    whitePlayerText(font, "Player 1 - White", 24),
    blackAdvantageText(font, "", 22),
    whiteAdvantageText(font, "", 22),

    blackPanel(Vector2f(600.0f, 80.0f)),
    whitePanel(Vector2f(600.0f, 80.0f)),
    moveHistoryPanel(Vector2f(380.0f, 600.0f)),

    newGameButton(Vector2f(130.0f, 40.0f)),
    newGameButtonText(font, "New Game", 18),

    saveGameButton(Vector2f(110.0f, 40.0f)),
    saveGameButtonText(font, "Save Game", 17),

    loadGameButton(Vector2f(110.0f, 40.0f)),
    loadGameButtonText(font, "Load Game", 17),

    moveHistoryTitle(font, "Move History", 28),
    moveNumberHeader(font, "No.", 18),
    whiteMoveHeader(font, "White", 18),
    blackMoveHeader(font, "Black", 18),

    confirmationOverlay(Vector2f(1010.0f, 800.0f)),
    confirmationPanel(Vector2f(450.0f, 200.0f)),

    confirmationTitle(font, "Load Saved Game?", 26),
    confirmationMessage(
        font,
        "Your current game will be replaced.",
        18
    ),

    confirmLoadButton(Vector2f(100.0f, 40.0f)),
    confirmLoadText(font, "Confirm", 18),

    cancelLoadButton(Vector2f(100.0f, 40.0f)),
    cancelLoadText(font, "Cancel", 18)
{
    // Player information
    blackPlayerText.setStyle(Text::Bold);
    blackPlayerText.setFillColor(Color::White);
    blackPlayerText.setPosition({ 20.0f, 20.0f });

    whitePlayerText.setStyle(Text::Bold);
    whitePlayerText.setFillColor(Color::White);
    whitePlayerText.setPosition({ 20.0f, 720.0f });

    blackAdvantageText.setStyle(Text::Bold);
    blackAdvantageText.setFillColor(Color::White);
    blackAdvantageText.setPosition({ 570.0f, 35.0f });

    whiteAdvantageText.setStyle(Text::Bold);
    whiteAdvantageText.setFillColor(Color::White);
    whiteAdvantageText.setPosition({ 570.0f, 725.0f });

    // Player panels
    blackPanel.setPosition({ 10.0f, 10.0f });
    blackPanel.setFillColor(Color(181, 139, 105));
    blackPanel.setOutlineColor(Color(100, 100, 100));
    blackPanel.setOutlineThickness(1.0f);

    whitePanel.setPosition({ 10.0f, 710.0f });
    whitePanel.setFillColor(Color(181, 139, 105));
    whitePanel.setOutlineColor(Color(100, 100, 100));
    whitePanel.setOutlineThickness(1.0f);

    moveHistoryPanel.setPosition({ 620.0f, 10.0f });
    moveHistoryPanel.setFillColor(Color(92, 64, 51));
    moveHistoryPanel.setOutlineColor(Color(100, 100, 100));
    moveHistoryPanel.setOutlineThickness(1.0f);

    // New Game button
    newGameButton.setPosition({ 620.0f, 620.0f });
    newGameButton.setFillColor(Color(181, 139, 105));
    newGameButton.setOutlineColor(Color(220, 220, 220));
    newGameButton.setOutlineThickness(1.0f);

    newGameButtonText.setStyle(Text::Bold);
    newGameButtonText.setFillColor(Color::White);
    newGameButtonText.setPosition({ 635.0f, 630.0f });

    // Save Game button
    saveGameButton.setPosition({ 760.0f, 620.0f });
    saveGameButton.setFillColor(Color(70, 120, 170));
    saveGameButton.setOutlineColor(Color(220, 220, 220));
    saveGameButton.setOutlineThickness(1.0f);

    saveGameButtonText.setStyle(Text::Bold);
    saveGameButtonText.setFillColor(Color::White);
    saveGameButtonText.setPosition({ 770.0f, 630.0f });

    // Load Game button
    loadGameButton.setPosition({ 880.0f, 620.0f });
    loadGameButton.setFillColor(Color(70, 120, 170));
    loadGameButton.setOutlineColor(Color(220, 220, 220));
    loadGameButton.setOutlineThickness(1.0f);

    loadGameButtonText.setStyle(Text::Bold);
    loadGameButtonText.setFillColor(Color::White);
    loadGameButtonText.setPosition({ 890.0f, 630.0f });

    // Move history headers
    moveHistoryTitle.setStyle(Text::Bold);
    moveHistoryTitle.setFillColor(Color::White);
    moveHistoryTitle.setPosition({ 640.0f, 15.0f });

    moveNumberHeader.setStyle(Text::Bold);
    moveNumberHeader.setFillColor(Color(220, 220, 220));
    moveNumberHeader.setPosition({ 640.0f, 65.0f });

    whiteMoveHeader.setStyle(Text::Bold);
    whiteMoveHeader.setFillColor(Color::White);
    whiteMoveHeader.setPosition({ 700.0f, 65.0f });

    blackMoveHeader.setStyle(Text::Bold);
    blackMoveHeader.setFillColor(Color::White);
    blackMoveHeader.setPosition({ 830.0f, 65.0f });

    // Load confirmation dialog
    confirmationOverlay.setFillColor(Color(0, 0, 0, 160));

    confirmationPanel.setPosition({ 280.0f, 300.0f });
    confirmationPanel.setFillColor(Color(70, 55, 45));
    confirmationPanel.setOutlineColor(Color(220, 217, 181));
    confirmationPanel.setOutlineThickness(2.0f);

    confirmationTitle.setStyle(Text::Bold);
    confirmationTitle.setFillColor(Color::White);
    confirmationTitle.setPosition({ 315.0f, 325.0f });

    confirmationMessage.setFillColor(Color(220, 220, 220));
    confirmationMessage.setPosition({ 315.0f, 375.0f });

    confirmLoadButton.setPosition({ 390.0f, 425.0f });
    confirmLoadButton.setFillColor(Color(70, 130, 80));

    confirmLoadText.setStyle(Text::Bold);
    confirmLoadText.setFillColor(Color::White);
    confirmLoadText.setPosition({ 405.0f, 433.0f });

    cancelLoadButton.setPosition({ 520.0f, 425.0f });
    cancelLoadButton.setFillColor(Color(150, 65, 65));

    cancelLoadText.setStyle(Text::Bold);
    cancelLoadText.setFillColor(Color::White);
    cancelLoadText.setPosition({ 540.0f, 433.0f });
}


void UI::drawPanelsAndHistory(
    RenderWindow& window,
    const GameData& game,
    int visibleRows
) {
    // Draw panels
    window.draw(blackPanel);
    window.draw(whitePanel);
    window.draw(moveHistoryPanel);

    // Draw buttons
    window.draw(newGameButton);
    window.draw(newGameButtonText);

    window.draw(saveGameButton);
    window.draw(saveGameButtonText);

    window.draw(loadGameButton);
    window.draw(loadGameButtonText);

    // Draw move history headers
    window.draw(moveHistoryTitle);
    window.draw(moveNumberHeader);
    window.draw(whiteMoveHeader);
    window.draw(blackMoveHeader);

    // Draw move history rows
    int endIndex = min(
        static_cast<int>(game.moveHistory.size()),
        game.historyScrollOffset + visibleRows
    );

    for (int i = game.historyScrollOffset; i < endIndex; i++) {
        int row = i - game.historyScrollOffset;

        float y = 100.0f + row * 24.0f;

        RectangleShape rowBackground({ 340.0f, 24.0f });
        rowBackground.setPosition({ 630.0f, y });

        if (i == static_cast<int>(game.moveHistory.size()) - 1) {
            rowBackground.setFillColor(Color(140, 105, 45));
        }
        else if (row % 2 == 0) {
            rowBackground.setFillColor(Color(110, 78, 62));
        }
        else {
            rowBackground.setFillColor(Color(92, 64, 51));
        }

        window.draw(rowBackground);

        Text moveNumber(font, to_string(i + 1) + ".", 17);
        moveNumber.setFillColor(Color(220, 220, 220));
        moveNumber.setPosition({ 640.0f, y });

        Text whiteMove(font, game.moveHistory[i].first, 17);
        whiteMove.setFillColor(Color::White);
        whiteMove.setPosition({ 700.0f, y });

        Text blackMove(font, game.moveHistory[i].second, 17);
        blackMove.setFillColor(Color::White);
        blackMove.setPosition({ 830.0f, y });

        window.draw(moveNumber);
        window.draw(whiteMove);
        window.draw(blackMove);
    }

    // Draw scrollbar
    int totalRows = static_cast<int>(game.moveHistory.size());

    if (totalRows > visibleRows) {
        int maxScroll = totalRows - visibleRows;

        RectangleShape scrollbarTrack({ 10.0f, 490.0f });
        scrollbarTrack.setPosition({ 980.0f, 100.0f });
        scrollbarTrack.setFillColor(Color(55, 40, 32));

        window.draw(scrollbarTrack);

        float thumbHeight = max(
            30.0f,
            490.0f * visibleRows / totalRows
        );

        float thumbTop =
            100.0f +
            (static_cast<float>(game.historyScrollOffset) / maxScroll) *
            (490.0f - thumbHeight);

        RectangleShape scrollbarThumb({ 10.0f, thumbHeight });
        scrollbarThumb.setPosition({ 980.0f, thumbTop });
        scrollbarThumb.setFillColor(Color(200, 170, 130));

        window.draw(scrollbarThumb);
    }
}

void UI::drawPlayerInfo(
    RenderWindow& window,
    const GameData& game
) {
    // Calculate material advantage
    int materialDifference = game.whiteMaterial - game.blackMaterial;

    if (materialDifference > 0) {
        whiteAdvantageText.setString("+" + to_string(materialDifference));
        blackAdvantageText.setString("");
    }
    else if (materialDifference < 0) {
        blackAdvantageText.setString("+" + to_string(-materialDifference));
        whiteAdvantageText.setString("");
    }
    else {
        whiteAdvantageText.setString("");
        blackAdvantageText.setString("");
    }

    // Draw player information
    window.draw(blackPlayerText);
    window.draw(whitePlayerText);

    window.draw(blackAdvantageText);
    window.draw(whiteAdvantageText);
}

void UI::drawLoadConfirmation(
    RenderWindow& window,
    bool showLoadConfirmation
) {
    if (showLoadConfirmation) {
        window.draw(confirmationOverlay);
        window.draw(confirmationPanel);

        window.draw(confirmationTitle);
        window.draw(confirmationMessage);

        window.draw(confirmLoadButton);
        window.draw(confirmLoadText);

        window.draw(cancelLoadButton);
        window.draw(cancelLoadText);
    }
}

