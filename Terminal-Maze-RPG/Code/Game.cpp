#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "Monster.h"
#include "Player.h"
#include "Section.h"
#include "Maze.h"
#include "Game.h"
using namespace std;

Game::Game() {
    //intializes game and plays the intro
    maze = Maze();
    crystalCount = 0;
    healthPoolCount = 0;
    intro();
}

void Game::battle() {
    //battles dont occur in the central section
    if (maze.getLocation(2) == 4) {
        return;
    }

    //gets the players steps and randomly determines if a battle should occur
    //the more steps, the higher the chance, up to a 100% chance at 700 steps
    int steps = player.getSteps();
    if (steps < 700) {
        int randNum = rand() % (70 - (steps/10));
        if (randNum != 0) {
            return;
        }
    }

    //prints out the screen with a monster and text, then waits for user input to continue
    for (int i = 0; i < 100; i++) {
        cout << endl;
    }
    string textString = "____________________________________________________________";
    for (int i = 0; i < 12; i++) {
        textString += "|                                                          |";
    }
    textString += "|               A Monster Is About To Attack!              |";
    textString += "|                                                          ||                 Press any key to continue                |";
    for (int i = 0; i < 13; i++) {
        textString += "|                                                          |";
    }
    textString += "************************************************************";
    string roomString = getRoomString(true);
    cout << "##################################################################################################################################" << endl;
    for (int i = 0; i < 30; i++) {
        cout << "#   " << textString.substr(i*60, 60) << "  " << roomString.substr(i*60, 60) << "   #" << endl;
    }
    for (int i = 0; i < 4; i++) {
        cout << "#                                                                                                                                #" << endl;
    }
    cout << "##################################################################################################################################" << endl;
    char waitKey;
    cin >> waitKey;

    //creates a new monster, and prepares for up to 27 rounds of battle
    Monster monster(steps);
    string battleText[27];
    for (int i = 0; i < 27; i++) {
        battleText[i] = " ";
    }

    //on each loop, a check happens to see if either the monster or player died
    //and if so the proper text is updated and the loop is ended
    //until then, the player attacks first, and the damage is recorded
    //then the monster attacks
    //if the monster was killed by the player, the monster doesn't attack
    //the attacks are a random number from 0 to their attack power - 1
    int damage;
    for (int i = 0; i < 27; i++) {
        if (i == 26 && player.getHealth() > 0) {
            battleText[i] += " The monster ran away!";
        }
        if (player.getHealth() == 0) {
            battleText[i] = battleText[i] + player.getName() + " has died... reviving!";
            i = 27;
            continue;
        }
        if (monster.getHealth() == 0) {
            battleText[i] += "The monster has been killed!";
            i = 27;
            continue;
        }
        damage = rand() % player.getPower();
        monster.setHealth(damage);
        battleText[i] = battleText[i] + "Player does " + to_string(damage) + " damage, ";
        if (monster.getHealth() == 0) {
            continue;
        }
        damage = rand() % monster.getPower();
        player.addDamage(damage);
        battleText[i] = battleText[i] + "the monster does " + to_string(damage) + " damage!";
    }

    //compiles all the individual rounds of battle into a text string
    textString = "____________________________________________________________";
    for (int i = 0; i < 27; i++) {
        textString = textString + "|" + battleText[i];
        for (int j = 0; j < 58 - battleText[i].length(); j++) {
            textString += " ";
        }
        textString += "|";
    }
    textString += "| Press any key to continue                                |************************************************************";
    
    //prints the battle results and screen, then waits for player input to continue
    for (int i = 0; i < 100; i++) {
        cout << endl;
    }
    cout << "##################################################################################################################################" << endl;
    for (int i = 0; i < 30; i++) {
        cout << "#   " << textString.substr(i*60, 60) << "  " << roomString.substr(i*60, 60) << "   #" << endl;
    }
    for (int i = 0; i < 4; i++) {
        cout << "#                                                                                                                                #" << endl;
    }
    cout << "##################################################################################################################################" << endl;
    cin >> waitKey;
    
    //if the player died, calls the player died function and creates a new maze
    if (player.getHealth() == 0) {
        player.die();
        maze.newMaze();
    }
}

void Game::intro() {
    //gets player input for first and last name
    //keeps asking for name until each name is 10 or less alphabetic characters
    //then creates a new player with the names
    string first;
    string last;
    bool correctFormat = false;
    while(!correctFormat) {
        correctFormat = true;
        cout << "Enter First Name: ";
        cin >> first;
        if (first.length() > 10) {
            cout << "First name must be 10 or less characters" << endl;
            correctFormat = false;
            continue;
        }
        for (int i = 0; i < first.length(); i++) {
            if (first[i] < 65 || first[i] < 65 || first[i] > 122 || (first[i] > 90 && first[i] < 97)) {
                cout << "First name must contain alphabetic characters only" << endl;
                correctFormat = false;
            }
        }
    }
    correctFormat = false;
    while(!correctFormat) {
        correctFormat = true;
        cout << "Enter Last Name: ";
        cin >> last;
        if (last.length() > 10) {
            cout << "Last name must be 10 or less characters" << endl;
            correctFormat = false;
            continue;
        }
        for (int i = 0; i < first.length(); i++) {
            if (last[i] < 65 || last[i] < 65 || last[i] > 122 || (last[i] > 90 && last[i] < 97)) {
                cout << "Last name must contain alphabetic characters only" << endl;
                correctFormat = false;
                continue;
            }
        }
    }
    player = Player(first, last);

    //greets the player and then opens the tutorial.txt file
    //prints out every line of the tutorial and waits for player input to continue
    cout << endl << endl << endl << endl << "  Hello, " << player.getName() << "!" << endl << endl;
    ifstream fin;
    fin.open("tutorial.txt");
    string line;
    while (getline(fin, line)) {
        cout << line << endl;
    }
    fin.close();
    char waitKey;
    cin >> waitKey;
}

//this is the main function that is recursively called until the player quits or wins
void Game::playGame() {
    //win conditions
    if (maze.getLocation(2) == 4 && crystalCount == 8) {
        outro();
        return;
    }

    drawScreen();

    char choice;
    cin >> choice;
    switch(choice) {
        //movements w,a,s,d
        //checks if move is valid, then moves, adds a step and calls this function again
        //if move is invalid then call this function again
        case 'w':
            if (maze.validMove(choice)) {
                maze.move(choice);
                player.addStep();
                playGame();
                break;
            } else {
                playGame();
                break;
            }
        case 'a':
            if (maze.validMove(choice)) {
                maze.move(choice);
                player.addStep();
                playGame();
                break;
            } else {
                playGame();
                break;
            }
        case 's':
            if (maze.validMove(choice)) {
                maze.move(choice);
                player.addStep();
                playGame();
                break;
            } else {
                playGame();
                break;
            }
        case 'd':
            if (maze.validMove(choice)) {
                maze.move(choice);
                player.addStep();
                playGame();
                break;
            } else {
                playGame();
                break;
            }

        //player wants to quit
        //double checks they want to quit
        case 'q':
            cout << "Are you sure you want to quit?  y - Quit     n - I've changed my mind" << endl;
            cin >> choice;
            if (choice != 'y') {
                playGame();
            }
            break;

        //draws the map
        case 'm':
            drawMap();
            playGame();
            break;

        //player is not cooperating so the function repeats until they do
        default:
            playGame();
            break;
    }
}

void Game::collectPowerUp() {
    //prints out collectable powerup image and awaits player input
    int tile = maze.getTile(maze.getLocation(1),maze.getLocation(0),maze.getLocation(2));
    string roomString = getRoomString(false);
    string textString = "____________________________________________________________";
    for (int i = 0; i < 12; i++) {
        textString += "|                                                          |";
    }
    if (tile == 2) {
        textString += "|                 You found a health pool!                 |";
    } else {
        textString += "|                You found a power crystal!                |";
    }
    textString += "|                                                          ||                 Press any key to collect                 |";
    for (int i = 0; i < 13; i++) {
        textString += "|                                                          |";
    }
    textString += "************************************************************";
    for (int i = 0; i < 100; i++) {
        cout << endl;
    }
    cout << "##################################################################################################################################" << endl;
    for (int i = 0; i < 30; i++) {
        cout << "#   " << textString.substr(i*60, 60) << "  " << roomString.substr(i*60, 60) << "   #" << endl;
    }
    for (int i = 0; i < 4; i++) {
        cout << "#                                                                                                                                #" << endl;
    }
    cout << "##################################################################################################################################" << endl;
    char waitKey;
    cin >> waitKey;

    //power up is either a health pool or crystal
    //updates the maze and player accordingly
    //then changes the tile to a standard one
    if (tile == 2) {
        healthPoolCount++;
        player.addHealth();
        if (maze.getLocation(2) < 4) {
            if (maze.getHealthPool(maze.getLocation(2)) == 2) {
                maze.setHealthPool(maze.getLocation(2), 1);
            } else {
                maze.setHealthPool(maze.getLocation(2), 1);
            }
        } else {
            if (maze.getHealthPool(maze.getLocation(2) - 1) == 2) {
                maze.setHealthPool(maze.getLocation(2) - 1, 1);
            } else {
                maze.setHealthPool(maze.getLocation(2) - 1, 1);
            }
        }
        
    } else {
        crystalCount++;
        player.addPower();
        if (maze.getLocation(2) < 4) {
            maze.setCrystal(maze.getLocation(2), false);
        } else {
            maze.setCrystal(maze.getLocation(2) - 1, false);
        }
        
    }
    maze.setTile(maze.getLocation(1),maze.getLocation(0),maze.getLocation(2),0);
}


string Game::getRoomString(bool battle) {
    string roomString;
    int roomType;
    int tile = maze.getTile(maze.getLocation(1), maze.getLocation(0),maze.getLocation(2));

    //if the player is in the central chamber, returns the its pitch black screen
    if (maze.getLocation(2) == 4) {
        roomString = "____________________________________________________________";
        for (int i = 0; i < 13; i++) {
            roomString += "|                                                          |";
        }
        roomString += "|                    It";
        roomString += "'";
        roomString += "s Pitch Black !                    |";
        for (int i = 0; i < 14; i++) {
            roomString += "|                                                          |";
        }
        roomString += "************************************************************";
        for (int i = 0; i < 30; i++) {
            cout << roomString.substr(i*60, 60) << endl;
        }
        return roomString;
    }

    //gets the room type: monster, health pool, crystal, resurrection stone, or regular
    if (battle) {
        roomType = 1;
    } else {
        switch (tile) {
        case 0:
            roomType = 0;
            break;
        case 2:
            roomType = 2;
            break;
        case 3:
            roomType = 4;
            break;
        case 4:
            roomType = 3;
            break;
        }
    }

    int roomNum = maze.getOrientation();

    //reads through the file with the rooms and puts the correct room type and orientation
    //room into the room string
    fstream fin;
    fin.open("rooms.txt");
    string line;
    for (int i = 0; i < 1200; i++) {
        getline(fin, line);
        if (i / 240 == roomType) {
            if ((i / 30) % 8 == roomNum) {
                roomString += line;
            }
        }
    }
    fin.close();

    //adds a compass to the lower right of the room drawing
    string newRoomString = roomString.substr(0,1380);
    string compass;
    switch(maze.getDirection()) {
        case 0:
            compass = "   _________   |  /    N    \\  | /     ^     \\ |( W    o    E )| \\           / |  \\    S    /  |****************";
            break;
        case 1:
            compass = "   _________   |  /    W    \\  | /           \\ |( S    o >  N )| \\           / |  \\    E    /  |****************";
            break;
        case 2:
            compass = "   _________   |  /    S    \\  | /           \\ |( E    o    W )| \\     v     / |  \\    N    /  |****************";
            break;
        case 3:
            compass = "   _________   |  /    E    \\  | /           \\ |( N  < o    S )| \\           / |  \\    W    /  |****************";
            break;
    }
    for (int i = 0; i < 7; i++) {
        newRoomString = newRoomString + roomString.substr(1380 + (i * 60), 44) + compass.substr(i*16,16);
    }
    return newRoomString;
}





string Game::getMapString() {
    string mapString = "";

    //adds an x and y offset depending on which quadrant
    //of the section the player is in
    int yOff;
    int xOff;
    if (maze.getLocation(0) < 15) {
        if (maze.getLocation(1) < 15) {
            xOff = 0;
            yOff = 0;
        } else {
            xOff = 15;
            yOff = 0;
        }
    } else {
        if (maze.getLocation(1) < 15) {
            xOff = 0;
            yOff = 15;
        } else {
            xOff = 15;
            yOff = 15;
        }
    }

    //reads each tile and adds the corresponding ascii characters to the map string
    for (int i = 0; i < 15; i++) {
        for (int k = 0; k < 2; k++) {
            for (int j = 0; j < 15; j++) {
                int tile = maze.getTile(j + xOff, i + yOff, maze.getLocation(2));
                if (maze.getFog(j + xOff, i + yOff, maze.getLocation(2))) {
                    mapString += "````";
                } else if (maze.getLocation(0) == i + yOff && maze.getLocation(1) == j + xOff) {
                    if (k == 0) {
                        mapString += "_o_|";
                    } else {
                        mapString += "/\\ |";
                    }
                } else if (tile == 0) {
                    mapString += "    ";
                } else if (tile == 1) {
                    mapString = mapString + "%" + "%" + "%" + "%";
                } else if (tile == 2) {
                    if (k == 0) {
                        mapString += "*HP*";
                    } else {
                        mapString += "\\__/";
                    }
                } else if (tile == 3) {
                    if (k == 0) {
                        mapString += "/RS\\";
                    } else {
                        mapString += "|@@|";
                    }
                } else if (tile == 4) {
                    if (k == 0) {
                        mapString += "*/\\*";
                    } else {
                        mapString += "*\\/*";
                    }
                }
            }
        }
    }

    //adds a border to the string
    string newMapString = "____________________________________________________________";
    for (int i = 60; i < 1740; i += 60) {
        newMapString = newMapString + "|" + mapString.substr(i + 1, 58) + "|";
    }
    newMapString += "************************************************************";
    return newMapString;
}



void Game::drawScreen() {
    //either player collects a powerup or has a chance of battle
    if (maze.getTile(maze.getLocation(1),maze.getLocation(0),maze.getLocation(2)) == 0) {
        battle();
    } else if (maze.getTile(maze.getLocation(1),maze.getLocation(0),maze.getLocation(2)) != 3) {
        collectPowerUp();
    }

    //takes the map string and room string a prints the screen
    for (int i = 0; i < 100; i++) {
        cout << endl;
    }
    for (int i = 0; i < 130; i++) {
        cout << "#";
    }
    cout << endl;
    string roomString = getRoomString(false);
    string mapString = getMapString();
    for (int i = 0; i < 30; i++) {
        cout << "#   " << mapString.substr(i*60, 60) << "  " << roomString.substr(i*60, 60) << "   #" << endl;
    }
    cout << "#                   Player Name:    " << player.getName();
    int x = player.getName().length();
    for (int i = 0; i < 39 - x; i++) {
        cout << " ";
    }
    cout << "Crystals Collected:    " << crystalCount << " / 8                          #" << endl;
    cout << "#                        Health:    " << player.getHealth();
    if (player.getHealth() < 100) {
        cout << " ";
    } else if (player.getHealth() < 10) {
        cout << "  ";
    }
    cout << " / " << player.getMaxHealth() << "                          Health Pools Collected:    " << healthPoolCount;
    if (healthPoolCount < 10) {
        cout << " ";
    }
    cout << " / 16                        #" << endl;
    cout << "#";
    for (int i = 0; i < 128; i++) {
        cout << " ";
    }
    cout << "#" << endl;
    cout << "#                          Move: w a s d                                            Map: m            Quit: q                    #" << endl;
    for (int i = 0; i < 130; i++) {
        cout << "#";
    }
    cout << endl;
}


void Game::drawMap() {
    //gets the maze
    char mazeString[1800];
    ifstream fin;
    fin.open("largeMaze.txt");
    string line;
    getline(fin,line);
    for (int i = 0; i < 1800; i++) {
        mazeString[i] = line[i];
    }

    //sets the crystals that are collected
    if (!maze.getCrystal(0)) {
        mazeString[371] = 'Y';
    }
    if (!maze.getCrystal(1)) {
        mazeString[389] = 'Y';
    }
    if (!maze.getCrystal(2)) {
        mazeString[407] = 'Y';
    }
    if (!maze.getCrystal(3)) {
        mazeString[911] = 'Y';
    }
    if (!maze.getCrystal(4)) {
        mazeString[947] = 'Y';
    }
    if (!maze.getCrystal(5)) {
        mazeString[1451] = 'Y';
    }
    if (!maze.getCrystal(6)) {
        mazeString[1469] = 'Y';
    }
    if (!maze.getCrystal(7)) {
        mazeString[1487] = 'Y';
    }

    //adds player current location to maze
    string initials = player.getInitials();
    switch(maze.getLocation(2)) {
        case 0:
            mazeString[490] = '*';
            mazeString[491] = initials[0];
            mazeString[492] = initials[1];
            mazeString[493] = '*';
            break;
        case 1:
            mazeString[508] = '*';
            mazeString[509] = initials[0];
            mazeString[510] = initials[1];
            mazeString[511] = '*';
            break;
        case 2:
            mazeString[526] = '*';
            mazeString[527] = initials[0];
            mazeString[528] = initials[1];
            mazeString[529] = '*';
            break;
        case 3:
            mazeString[1030] = '*';
            mazeString[1031] = initials[0];
            mazeString[1032] = initials[1];
            mazeString[1033] = '*';
            break;
        case 4:
            mazeString[1048] = '*';
            mazeString[1049] = initials[0];
            mazeString[1050] = initials[1];
            mazeString[1051] = '*';
            break;
        case 5:
            mazeString[1066] = '*';
            mazeString[1067] = initials[0];
            mazeString[1068] = initials[1];
            mazeString[1069] = '*';
            break;
        case 6:
            mazeString[1570] = '*';
            mazeString[1571] = initials[0];
            mazeString[1572] = initials[1];
            mazeString[1573] = '*';
            break;
        case 7:
            mazeString[1588] = '*';
            mazeString[1589] = initials[0];
            mazeString[1590] = initials[1];
            mazeString[1591] = '*';
            break;
        case 8:
            mazeString[1606] = '*';
            mazeString[1607] = initials[0];
            mazeString[1608] = initials[1];
            mazeString[1609] = '*';
            break;
    }

    //gets the map of the entire section
    string sectionString = "";
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 30; j++) {
            int tile = maze.getTile(j, i, maze.getLocation(2));
            if (maze.getLocation(0) == i && maze.getLocation(1) == j) {
                sectionString += initials;
            } else if (maze.getFog(j,i,maze.getLocation(2))) {
                sectionString += "``";
            } else if (tile == 0) {
                sectionString += "  ";
            } else if (tile == 1) {
                sectionString = sectionString + "%" + "%";
            } else if (tile == 2) {
                sectionString += "HP";
            } else if (tile == 3) {
                sectionString += "RS";
            } else if (tile == 4) {
                sectionString += "CR";
            }
        }
    }

    //prints the screen
    for (int i = 0; i < 100; i++) {
        cout << endl;
    }
    for (int i = 0; i < 130; i++) {
        cout << "#";
    }
    cout << endl;
    for (int i = 0; i < 30; i++) {
        cout << "#   " << sectionString.substr(i*60, 60) << "  ";
        for (int j = 0; j < 60; j++) {
            cout << mazeString[(i*60) + j];
        }
        cout << "   #" << endl;
    }
    cout << "#                   Player Name:    " << player.getName();
    int x = player.getName().length();
    for (int i = 0; i < 39 - x; i++) {
        cout << " ";
    }
    cout << "Crystals Collected:    " << crystalCount << " / 8                          #" << endl;
    cout << "#                        Health:    " << player.getHealth();
    if (player.getHealth() < 100) {
        cout << " ";
    } else if (player.getHealth() < 10) {
        cout << "  ";
    }
    cout << " / " << player.getMaxHealth() << "                          Health Pools Collected:    " << healthPoolCount;
    if (healthPoolCount < 10) {
        cout << " ";
    }
    cout << " / 16                        #" << endl;
    cout << "#";
    for (int i = 0; i < 128; i++) {
        cout << " ";
    }
    cout << "#" << endl;
    cout << "#                                                           Close Map: m                                                         #" << endl;
    for (int i = 0; i < 130; i++) {
        cout << "#";
    }
    cout << endl;

    //asks user to close map
    char choice;
    cin >> choice;
    if (choice != 'm') {
        drawMap();
    }
}

void Game::outro() {
    //prints out win message and assigns rank based on how many health pools were collected
    for (int i = 0; i < 100; i++) {
        cout << endl;
    }
    cout << "Congratulations! You have collected all power crystals and conquered the maze!" << endl << endl;
    cout << "For your performance in the maze, collecting " << healthPoolCount << " / 16 health pools, you have been granted the rank of:" << endl << endl;
    if (healthPoolCount < 4) {
        cout << "Talentless Wretch!" << endl;
    } else if (healthPoolCount < 8) {
        cout << "Adequate Spellcaster!" << endl;
    } else if (healthPoolCount < 11) {
        cout << "Adept Sorcerer!" << endl;
    } else if (healthPoolCount < 14) {
        cout << "Prodigal Conjurer!" << endl;
    } else if (healthPoolCount < 16) {
        cout << "Master Wizard!" << endl;
    } else {
        cout << "Grand Archmage!" << endl;
    }
    cout << endl << "Thanks for Playing!" << endl << endl;
}
