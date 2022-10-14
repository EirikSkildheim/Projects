#include <iostream>
#include "Section.cpp"
#include "Maze.cpp"
#include "Player.cpp"
#include "Monster.cpp"
#include "Game.cpp"
using namespace std;

int main() {
    //plays the game
    Game game = Game();
    game.playGame();
}