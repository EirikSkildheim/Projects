#ifndef MAZE_H
#define MAZE_H

#include "Section.h"
using namespace std;


class Maze {

    public:
        Maze();
        void move(char direction);
        bool validMove(char direction);
        void newMaze();
        bool getCrystal(int index);
        int getHealthPool(int index);
        int getLocation(int index);
        int getDirection();
        void setCrystal(int index, bool value);
        void setHealthPool(int index, bool value);
        void setLocation(int index, int value);
        void setDirection(int value);
        int getTile(int x, int y, int section);
        void setTile(int x, int y, int section, int value);
        bool getFog(int x, int y, int section);
        int getOrientation();

        void moveTest(int crystals, int healths);

    private:
        Section sections[9];
        bool crystals[8];
        int healthPools[8];
        int location[3];
        int direction;
        int randomnessCounter;

};
#endif