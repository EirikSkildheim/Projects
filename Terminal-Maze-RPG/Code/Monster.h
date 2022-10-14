#ifndef MONSTER_H
#define MONSTER_H

using namespace std;

class Monster {

    public:

        Monster(int steps);
        int getMaxHealth();
        int getHealth();
        int getPower();
        void setHealth(int damage);

    private:
        int maxHealth;
        int health;
        int power;

};

#endif