#include "Monster.h"
using namespace std;

Monster::Monster(int steps) {
    //initializes attack power and health of monster based on steps since last revival
    power = 2 + (steps / 30);
    maxHealth = 100 + (steps/5);
    health = maxHealth;
}

int Monster::getMaxHealth() {
    return maxHealth;
}

int Monster::getHealth() {
    return health;
}

int Monster::getPower() {
    return power;
}

void Monster::setHealth(int damage) {
    if (health - damage < 0) {
        health = 0;
    } else {
        health -= damage;
    }
}