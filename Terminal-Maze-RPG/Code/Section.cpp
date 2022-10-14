#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <ctime>
#include "Section.h"
#include "split.cpp"

using namespace std;

Section::Section() {}

//creates a section made up of a 2x2 of smaller maze pieces that are 15 x 15
//random number generation ensures a different maze every time
//the maze pieces are read from a txt file
Section::Section(int roomType, int healthPools, bool crystal, int randCounter) {
    //sets entire fog map to true
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 30; j++) {
            fogMap[i][j] = true;
        }
    }

    //special case for center of maze
    if (roomType == 4) {
        for (int i = 0; i < 30; i++) {
            for (int j = 0; j < 30; j++) {
                if(i == 0 || i == 29 || j == 0 || j == 29) {
                    map[i][j] = 1;
                } else {
                    map[i][j] = 0;
                }
            }
        }

    //all the regular sections
    } else {
        vector<int> rooms;
        ifstream fin;
        fin.open("maze.txt");
        bool pieces[12][15][15];
        string line;
        string info[15];

        //open the maze.txt file and reads through all 180 lines
        //maze.txt contains a boolean array of t or f values
        //representing 12 different 15x15 mazes
        //it stores the values in a boolean array pieces
        //for pieces[i][j][k]:
        //i represents one of the 12 maze pieces
        //j represents the y value
        //k represents the x value
        for (int i = 0; i < 180; i++) {
            getline(fin, line);
            split(line, ',', info, 15);
            for (int j = 0; j < 15; j++) {
                if (info[j] == "t") {
                    pieces[i/15][i%15][j] = true;
                } else {
                    pieces[i/15][i%15][j] = false;
                }
            }
        }

        //seeds the rand function with time, the roomType, and a random counter
        //to ensure no repeats in the same game
        //the variables a,b,c,d each choose a random piece from the 12
        //a nested loop is used to map the 4 individual sections
        //from the 12x15x15 pieces array
        //onto the 30x30 map array
        srand((unsigned int)(time(NULL) + roomType + randCounter));
        int a = rand() % 12;
        int b = rand() % 12;
        int c = rand() % 12;
        int d = rand() % 12;
        for (int i = 0; i < 30; i++) {
            for (int j = 0; j < 30; j++) {
                if (i < 15) {
                    if (j < 15) {
                        if (pieces[a][i][j]) {
                            map[i][j] = 1;
                        } else {
                            map[i][j] = 0;
                        }
                    } else {
                        if (pieces[b][i][j%15]) {
                            map[i][j] = 1;
                        } else {
                            map[i][j] = 0;
                        }
                    }
                } else {
                    if (j < 15) {
                        if (pieces[c][i%15][j]) {
                            map[i][j] = 1;
                        } else {
                            map[i][j] = 0;
                        }
                    } else {
                        if (pieces[d][i%15][j%15]) {
                            map[i][j] = 1;
                        } else {
                            map[i][j] = 0;
                        }
                    }
                }
            }
        }
        fin.close();

        //each 15 x 15 piece contains 49 spots guaranteed to not be a wall
        //one of these spots is chosen and then one of the four pieces is chosen
        //the y value of the tile within the section is position/7
        //the x value is position%7
        //depending on which of the 4 pieces is chosen
        //an x and/or y adjustment of 15 is added
        //the resluting tile is changed to 4, the value of a crystal tile
        if (crystal) {
            int position = rand() % 49;
            int room = rand() % 4;
            switch(room) {
                case 0:
                    map[(((position/7) + 1) * 2) -1][(((position%7) + 1) * 2) -1] = 4;
                    break;
                case 1:
                    map[(((position/7) + 1) * 2) -1 ][(((position%7) + 1) * 2) -1 +15] = 4;
                    break;
                case 2:
                    map[(((position/7) + 1) * 2) -1 +15][(((position%7) + 1) * 2) -1] = 4;
                    break;
                case 3:
                    map[(((position/7) + 1) * 2) -1 +15][(((position%7) + 1) * 2) -1 +15] = 4;
                    break;
            }
            rooms.push_back(room);
        }

        //the same algorithm is used as above with two new additions
        //first, there can be zero, one or two health pools instead of just zero or one crystals
        //the function loops through wither once or twice
        //second, to ensure the same piece doesn't have two special tiles
        //a vector is used where any piece already containing a tile
        //has its value added to the vector
        //a while loop keeps drawing new random values until an open one is found
        if (healthPools > 0) {
            for (int i = 0; i < healthPools; i++) {
                int position = rand() % 49;
                int room = rand() % 4;
                bool roomTaken = true;
                while(roomTaken) {
                    roomTaken = false;
                    for (int i = 0; i < rooms.size(); i++) {
                        if (rooms.at(i) == room) {
                            roomTaken = true;
                            room = rand() % 4;
                        }
                    }
                }
                switch(room) {
                    case 0:
                        map[(((position/7) + 1) * 2) -1][(((position%7) + 1) * 2) -1] = 2;
                        break;
                    case 1:
                        map[(((position/7) + 1) * 2) -1 ][(((position%7) + 1) * 2) -1 +15] = 2;
                        break;
                    case 2:
                        map[(((position/7) + 1) * 2) -1 +15][(((position%7) + 1) * 2) -1] = 2;
                        break;
                    case 3:
                        map[(((position/7) + 1) * 2) -1 +15][(((position%7) + 1) * 2) -1 +15] = 2;
                        break;
                }
                rooms.push_back(room);
            }
        }

        //this algorithm is the same as health pool, except it only runs once
        //and it runs every time, it does not have a zero value
        //this sets the resurrection stone
        //the resurrection stone coordinates are saved in a variable
        int position = rand() % 49;
        int room = rand() % 4;
        bool roomTaken = true;
        while(roomTaken) {
            roomTaken = false;
            for (int i = 0; i < rooms.size(); i++) {
                if (rooms.at(i) == room) {
                    roomTaken = true;
                    room = rand() % 4;
                }
            }
        }
        switch(room) {
            case 0:
                map[(((position/7) + 1) * 2) -1][(((position%7) + 1) * 2) -1] = 3;
                resurrectionStone[0] = (((position/7) + 1) * 2) -1;
                resurrectionStone[1] = (((position%7) + 1) * 2) -1;
                break;
            case 1:
                map[(((position/7) + 1) * 2) -1 ][(((position%7) + 1) * 2) -1 +15] = 3;
                resurrectionStone[0] = (((position/7) + 1) * 2) -1;
                resurrectionStone[1] = (((position%7) + 1) * 2) -1 +15;
                break;
            case 2:
                map[(((position/7) + 1) * 2) -1 +15][(((position%7) + 1) * 2) -1] = 3;
                resurrectionStone[0] = (((position/7) + 1) * 2) -1 +15;
                resurrectionStone[1] = (((position%7) + 1) * 2) -1;
                break;
            case 3:
                map[(((position/7) + 1) * 2) -1 +15][(((position%7) + 1) * 2) -1 +15] = 3;
                resurrectionStone[0] = (((position/7) + 1) * 2) -1 +15;
                resurrectionStone[1] = (((position%7) + 1) * 2) -1 +15;
                break;
        }

        //the pieces by default have four sides open
        //these set the values of the outside to a wall
        map[0][7] = 1;
        map[0][22] = 1;
        map[7][0] = 1;
        map[22][0] = 1;
        map[29][7] = 1;
        map[29][22] = 1;
        map[7][29] = 1;
        map[22][29] = 1;
    }
}

bool Section::getFog(int x, int y) {
    return fogMap[y][x];
}

int Section::getTile(int x, int y) {
    return map[y][x];
}

void Section::setFog(int x, int y, bool value) {
    fogMap[y][x] = value;
}

void Section::setTile(int x, int y, int value) {
    map[y][x] = value;
}

int Section::getResurrectionStone(int index) {
    return resurrectionStone[index];
}