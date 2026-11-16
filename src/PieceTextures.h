#pragma once // Prevent duplicate

#include <SFML/Graphics.hpp>
#include "Board.h"

using namespace sf;
using namespace std;

struct PieceTextures
{
	Texture whiteKing;
	Texture whiteQueen;
	Texture whiteRook;
	Texture whiteBishop;
	Texture whiteKnight;
	Texture whitePawn;

	Texture blackKing;
	Texture blackQueen;
	Texture blackRook;
	Texture blackBishop;
	Texture blackKnight;
	Texture blackPawn;
};

void loadPieceTextures(PieceTextures &textures);
Texture &getPieceTexture(char piece, PieceTextures &textures);
void drawPieces(
	RenderWindow &window,
	Board &board,
	PieceTextures &textures,
	float squareSize,
	float startX,
	float startY);