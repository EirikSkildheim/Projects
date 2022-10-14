#include <iostream>
#include <cmath>
#include <ctime>
#include "Maze.h"
using namespace std;

Maze::Maze() {
    //sets intial maze values and calls newMaze function
    for (int i = 0; i < 8; i++) {
        crystals[i] = true;
        healthPools[i] = 2;
    }
    randomnessCounter = 0;
    newMaze();
}

void Maze::newMaze() {
    //gets a random spawning section but not four
    srand((unsigned int)(time(NULL) + randomnessCounter));
    int section = rand() % 9;
    if (section == 4) {
        section++;
    }

    //for each section, a new section is created with the current game stats
    //then any walls that adjoin another section and should be open
    //are changed to the open value
    //then the randomness counter is incremented
    sections[0] = Section(0, healthPools[0], crystals[0], randomnessCounter);
    sections[0].setTile(7,29,0);
    sections[0].setTile(22,29,0);
    sections[0].setTile(29,7,0);
    sections[0].setTile(29,22,0);
    randomnessCounter++;
    sections[1] = Section(1, healthPools[1], crystals[1], randomnessCounter);
    sections[1].setTile(0,7,0);
    sections[1].setTile(0,22,0);
    sections[1].setTile(29,7,0);
    sections[1].setTile(29,22,0);
    sections[1].setTile(7,29,0);
    sections[1].setTile(22,29,0);
    randomnessCounter++;
    sections[2] = Section(2, healthPools[2], crystals[2], randomnessCounter);
    sections[2].setTile(7,29,0);
    sections[2].setTile(22,29,0);
    sections[2].setTile(0,7,0);
    sections[2].setTile(0,22,0);
    randomnessCounter++;
    sections[3] = Section(3, healthPools[3], crystals[3], randomnessCounter);
    sections[3].setTile(7,0,0);
    sections[3].setTile(22,0,0);
    sections[3].setTile(29,7,0);
    sections[3].setTile(29,22,0);
    sections[3].setTile(7,29,0);
    sections[3].setTile(22,29,0);
    randomnessCounter++;
    sections[4] = Section(4, 0, false, 0);
    sections[4].setTile(7,0,0);
    sections[4].setTile(22,0,0);
    sections[4].setTile(0,7,0);
    sections[4].setTile(0,22,0);
    sections[4].setTile(7,29,0);
    sections[4].setTile(22,29,0);
    sections[4].setTile(29,7,0);
    sections[4].setTile(29,22,0);
    sections[5] = Section(5, healthPools[4], crystals[4], randomnessCounter);
    sections[5].setTile(7,0,0);
    sections[5].setTile(22,0,0);
    sections[5].setTile(0,7,0);
    sections[5].setTile(0,22,0);
    sections[5].setTile(7,29,0);
    sections[5].setTile(22,29,0);
    randomnessCounter++;
    sections[6] = Section(6, healthPools[5], crystals[5], randomnessCounter);
    sections[6].setTile(7,0,0);
    sections[6].setTile(22,0,0);
    sections[6].setTile(29,7,0);
    sections[6].setTile(29,22,0);
    randomnessCounter++;
    sections[7] = Section(7, healthPools[6], crystals[6], randomnessCounter);
    sections[7].setTile(0,7,0);
    sections[7].setTile(0,22,0);
    sections[7].setTile(29,7,0);
    sections[7].setTile(29,22,0);
    sections[7].setTile(7,0,0);
    sections[7].setTile(22,0,0);
    randomnessCounter++;
    sections[8] = Section(8, healthPools[7], crystals[7], randomnessCounter);
    sections[8].setTile(7,0,0);
    sections[8].setTile(22,0,0);
    sections[8].setTile(0,7,0);
    sections[8].setTile(0,22,0);
    randomnessCounter++;

    //the players location is set to the location of the resurrection stone
    //in a randomly chosen section
    location[2] = section;
    location[0] = sections[section].getResurrectionStone(0);
    location[1] = sections[section].getResurrectionStone(1);

    //the fog around the player is lifted
    sections[location[2]].setFog(location[1] -1,location[0] -1,false);
    sections[location[2]].setFog(location[1] -1,location[0],false);
    sections[location[2]].setFog(location[1] -1,location[0] +1,false);
    sections[location[2]].setFog(location[1] +1,location[0] -1,false);
    sections[location[2]].setFog(location[1] +1,location[0],false);
    sections[location[2]].setFog(location[1] +1,location[0] +1,false);
    sections[location[2]].setFog(location[1],location[0] -1,false);
    sections[location[2]].setFog(location[1],location[0],false);
    sections[location[2]].setFog(location[1],location[0] +1,false);

    //sets the initial orientation of the player based on the walls around them
    if (validMove('w')) {
        direction = 0;
    } else if (validMove('a')) {
        direction = 1;
    } else if (validMove('s')) {
        direction = 2;
    } else if (validMove('d')) {
        direction = 3;
    }
}


void Maze::move(char choice) {
    //depending on the direction of movement, the direction is updated
    //then if the movement is not moving to the edge of the screen
    //the fog is set to false around the players intended position
    //and the player moves
    //if the player is moving to the edge of the screen, there is no fog
    //to clear so they just move
    //if the player is at the edge of the screen, they are moved to the opposite edge of the adjacent tile
    //and the fog is removed accordingly
    switch(choice) {
        case 'w':
        direction = 0;
            if (location[0] > 1) {
                sections[location[2]].setFog(location[1] -1,location[0] - 2,false);
                sections[location[2]].setFog(location[1],location[0] - 2,false);
                sections[location[2]].setFog(location[1] +1,location[0] - 2,false);
                location[0]--;
            } else if (location[0] == 1) {
                location[0]--;
            } else {
                switch(location[2]) {
                    case 3:
                        location[2] = 0;
                        break;
                    case 4:
                        location[2] = 1;
                        break;
                    case 5:
                        location[2] = 2;
                        break;
                    case 6:
                        location[2] = 3;
                        break;
                    case 7:
                        location[2] = 4;
                        break;
                    case 8:
                        location[2] = 5;
                        break;
                }
                location[0] = 29;
                sections[location[2]].setFog(location[1] -1,location[0] - 1,false);
                sections[location[2]].setFog(location[1],location[0] - 1,false);
                sections[location[2]].setFog(location[1] +1,location[0] - 1,false);
                sections[location[2]].setFog(location[1] -1,location[0],false);
                sections[location[2]].setFog(location[1],location[0],false);
                sections[location[2]].setFog(location[1] +1,location[0],false);
            }
            break;
        
        case 'd':
            direction = 3;
            if (location[1] < 28) {
                sections[location[2]].setFog(location[1] +2,location[0] -1,false);
                sections[location[2]].setFog(location[1] +2,location[0],false);
                sections[location[2]].setFog(location[1] +2,location[0] +1,false);
                location[1]++;
            } else if (location[1] == 28) {
                location[1]++;
            } else {
                switch(location[2]) {
                    case 0:
                        location[2] = 1;
                        break;
                    case 3:
                        location[2] = 4;
                        break;
                    case 6:
                        location[2] = 7;
                        break;
                    case 1:
                        location[2] = 2;
                        break;
                    case 4:
                        location[2] = 5;
                        break;
                    case 7:
                        location[2] = 8;
                        break;
                }
                location[1] = 0;
                sections[location[2]].setFog(location[1] +1,location[0] -1,false);
                sections[location[2]].setFog(location[1] +1,location[0],false);
                sections[location[2]].setFog(location[1] +1,location[0] +1,false);
                sections[location[2]].setFog(location[1],location[0] -1,false);
                sections[location[2]].setFog(location[1],location[0],false);
                sections[location[2]].setFog(location[1],location[0] +1,false);
            }
            break;

        case 's':
            direction = 2;
            if (location[0] < 28) {
                sections[location[2]].setFog(location[1] -1,location[0] + 2,false);
                sections[location[2]].setFog(location[1],location[0] + 2,false);
                sections[location[2]].setFog(location[1] +1,location[0] + 2,false);
                location[0]++;
            } else if (location[0] == 28) {
                location[0]++;
            } else {
                switch(location[2]) {
                    case 0:
                        location[2] = 3;
                        break;
                    case 1:
                        location[2] = 4;
                        break;
                    case 2:
                        location[2] = 5;
                        break;
                    case 3:
                        location[2] = 6;
                        break;
                    case 4:
                        location[2] = 7;
                        break;
                    case 5:
                        location[2] = 8;
                        break;
                }
                location[0] = 0;
                sections[location[2]].setFog(location[1] -1,location[0] + 1,false);
                sections[location[2]].setFog(location[1],location[0] + 1,false);
                sections[location[2]].setFog(location[1] +1,location[0] + 1,false);
                sections[location[2]].setFog(location[1] -1,location[0],false);
                sections[location[2]].setFog(location[1],location[0],false);
                sections[location[2]].setFog(location[1] +1,location[0],false);
            }
            break;
        
        case 'a':
            direction = 1;
            if (location[1] > 1) {
                sections[location[2]].setFog(location[1] -2,location[0] -1,false);
                sections[location[2]].setFog(location[1] -2,location[0],false);
                sections[location[2]].setFog(location[1] -2,location[0] +1,false);
                location[1]--;
            } else if (location[1] == 1) {
                location[1]--;
            } else {
                switch(location[2]) {
                    case 1:
                        location[2] = 0;
                        break;
                    case 4:
                        location[2] = 3;
                        break;
                    case 7:
                        location[2] = 6;
                        break;
                    case 2:
                        location[2] = 1;
                        break;
                    case 5:
                        location[2] = 4;
                        break;
                    case 8:
                        location[2] = 7;
                        break;
                }
                location[1] = 29;
                sections[location[2]].setFog(location[1] -1,location[0] -1,false);
                sections[location[2]].setFog(location[1] -1,location[0],false);
                sections[location[2]].setFog(location[1] -1,location[0] +1,false);
                sections[location[2]].setFog(location[1],location[0] -1,false);
                sections[location[2]].setFog(location[1],location[0],false);
                sections[location[2]].setFog(location[1],location[0] +1,false);
            }
            break;
    }
}


bool Maze::validMove(char directionChoice) {
    //this function takes the direction the player wants to move in and checks
    //whether it is a valid move
    //if there is no wall in the way the move is valid
    //if the player is at the edge of a section moving to another
    //the move is valid
    switch(directionChoice) {
        case 'w':
            if (location[0] == 0) {
                return true;
            } else if (sections[location[2]].getTile(location[1], location[0] -1) == 1) {
                return false;
            } else {
                return true;
            }

        case 'a':
            if (location[1] == 0) {
                return true;
            } else if (sections[location[2]].getTile(location[1] -1, location[0]) == 1) {
                return false;
            } else {
                return true;
            }

        case 's':
            if (location[0] == 29) {
                return true;
            } else if (sections[location[2]].getTile(location[1], location[0] +1) == 1) {
                return false;
            } else {
                return true;
            }

        case 'd':
            if (location[1] == 29) {
                return true;
            } else if (sections[location[2]].getTile(location[1] +1, location[0]) == 1) {
                return false;
            } else {
                return true;
            }

        default:
            return false;
    }
}


bool Maze::getCrystal(int index) {
    return crystals[index];
}

int Maze::getHealthPool(int index) {
    return healthPools[index];
}

int Maze::getLocation(int index) {
    //location is an array with three values: y, x, section
    return location[index];
}

int Maze::getDirection() {
    return direction;
}

void Maze::setCrystal(int index, bool value) {
    crystals[index] = value;
}

void Maze::setHealthPool(int index, bool value) {
    healthPools[index] = value;
}

void Maze::setLocation(int index, int value) {
    location[index] = value;
}

void Maze::setDirection(int value) {
    direction = value;
}

int Maze::getTile(int x, int y, int section) {
    return sections[section].getTile(x,y);
}

void Maze::setTile(int x, int y, int section, int value) {
    sections[section].setTile(x,y,0);
}

bool Maze::getFog(int x, int y, int section) {
    return sections[section].getFog(x,y);
}

int Maze::getOrientation() {
    //depending on the direction the player is heading in, it checks
    //if the path to the left is open, the path to the right is open
    //and the path ahead is open, then returns a room number
    //the values below are read yes or no in the order: left,straight,right = room number
    //n,n,n = 0
    //n,n,y = 1
    //n,y,n = 2
    //n,y,y = 3
    //y,n,n = 4
    //y,n,y = 5
    //y,y,n = 6
    //y,y,y = 7
    //if the player is about to move to another section it retuns 2
    switch(direction) {
        case 0:
            //edge of screen
            if (location[0] == 0) {
                return 2;
            }
            //left is closed
            if (sections[location[2]].getTile(location[1] - 1, location[0]) == 1) {
                //straight is closed
                if (sections[location[2]].getTile(location[1], location[0] - 1) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] + 1, location[0]) == 1) {
                        return 0;
                    } else {
                        return 1;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] + 1, location[0]) == 1) {
                        return 2;
                    } else {
                        return 3;
                    }
                }
            } else {
                //straight is closed
                if (sections[location[2]].getTile(location[1], location[0] - 1) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] + 1, location[0]) == 1) {
                        return 4;
                    } else {
                        return 5;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] + 1, location[0]) == 1) {
                        return 6;
                    } else {
                        return 7;
                    }
                }
            }

        case 1:
            if (location[1] == 0) {
                return 2;
            }
            //left is closed
            if (sections[location[2]].getTile(location[1], location[0] + 1) == 1) {
                //straight is closed
                if (sections[location[2]].getTile(location[1] - 1, location[0]) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] - 1) == 1) {
                        return 0;
                    } else {
                        return 1;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] - 1) == 1) {
                        return 2;
                    } else {
                        return 3;
                    }
                }
            } else {
                //straight is closed
                if (sections[location[2]].getTile(location[1] - 1, location[0]) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] - 1) == 1) {
                        return 4;
                    } else {
                        return 5;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] - 1) == 1) {
                        return 6;
                    } else {
                        return 7;
                    }
                }
            }


        case 2:
            if (location[0] == 29) {
                return 2;
            }
            //left is closed
            if (sections[location[2]].getTile(location[1] + 1, location[0]) == 1) {
                //straight is closed
                if (sections[location[2]].getTile(location[1], location[0] + 1) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] - 1, location[0]) == 1) {
                        return 0;
                    } else {
                        return 1;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] - 1, location[0]) == 1) {
                        return 2;
                    } else {
                        return 3;
                    }
                }
            } else {
                //straight is closed
                if (sections[location[2]].getTile(location[1], location[0] + 1) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] - 1, location[0]) == 1) {
                        return 4;
                    } else {
                        return 5;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1] - 1, location[0]) == 1) {
                        return 6;
                    } else {
                        return 7;
                    }
                }
            }


        case 3:
            if (location[1] == 29) {
                return 2;
            }
            //left is closed
            if (sections[location[2]].getTile(location[1], location[0] - 1) == 1) {
                //straight is closed
                if (sections[location[2]].getTile(location[1] + 1, location[0]) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] + 1) == 1) {
                        return 0;
                    } else {
                        return 1;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] + 1) == 1) {
                        return 2;
                    } else {
                        return 3;
                    }
                }
            } else {
                //straight is closed
                if (sections[location[2]].getTile(location[1] + 1, location[0]) == 1) {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] + 1) == 1) {
                        return 4;
                    } else {
                        return 5;
                    }
                } else {
                    //right is closed
                    if (sections[location[2]].getTile(location[1], location[0] + 1) == 1) {
                        return 6;
                    } else {
                        return 7;
                    }
                }
            }
    }
    return -1;
}
