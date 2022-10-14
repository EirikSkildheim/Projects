#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
using namespace std;

class Player {

    public:
        Player();
        Player(string firstName, string lastName);
        int getMaxHealth();
        int getHealth();
        int getPower();
        int getSteps();
        string getName();
        string getInitials();
        void addPower();
        void addHealth();
        void addStep();
        void addDamage(int damage);
        void die();

    private:
        string name;
        string initials;
        int maxHealth;
        int health;
        int power;
        int steps;
};

#endif