#include <iostream>
#include <string>
#include "Player.h"
#include "toLower.cpp"

using namespace std;

    Player::Player(){};
    Player::Player(string firstName, string lastName) {
        //takes first and last name and gets the initials and full name,
        //then creates a base stats character
        initials = "";
        firstName = toLower(firstName);
        lastName = toLower(lastName);
        initials += (firstName[0] -32);
        initials += (lastName[0] -32);
        name = initials[0] + firstName.substr(1, firstName.length() - 1) + " " + initials[1] + lastName.substr(1, lastName.length() - 1);
        maxHealth = 100;
        health = 100;
        power = 20;
        steps = 0;
    }

    int Player::getMaxHealth(){
        return maxHealth;
    }

    int Player::getHealth(){
        return health;
    }

    int Player::getPower(){
        return power;
    }

    int Player::getSteps(){
        return steps;
    }

    string Player::getName() {
        return name;
    }

    string Player::getInitials() {
        return initials;
    }

    void Player::addPower(){
        power += 10;
    }

    void Player::addHealth(){
        maxHealth+= 25;
        health += 25;
    }

    void Player::addDamage(int damage){
        if (health - damage < 0) {
            health = 0;
        } else {
            health -= damage;
        }
    }

    void Player::addStep(){
        steps++;
    }
    
    void Player::die(){
        steps = 0;
        health = maxHealth;
    }