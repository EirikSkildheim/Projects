#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include "Monster.h"
#include "Player.h"
#include "Section.h"
#include "Maze.h"
using namespace std;

class Game {

    public:
        Game();
        void battle();
        void drawScreen();
        void drawMap();
        void playGame();
        void intro();
        void collectPowerUp();
        string getRoomString(bool battle);
        string getMapString();
        void outro();

    private:
        Maze maze;
        int crystalCount;
        int healthPoolCount;
        Player player;
};

#endif