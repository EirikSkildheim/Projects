#include <iostream>
using namespace std;

string toLower(string str) {
    string newstr = "";
    for (int i = 0; i < str.length(); i++) {
        if (str[i] < 91 && 64 < str[i]) {
            newstr += (str[i] + 32);
        } else {
            newstr += str[i];
        }
    }
    return newstr;
}