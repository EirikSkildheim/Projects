#include <iostream>
using namespace std;

int split(string str, char del, string arr[], int size) {
    if (str.length() == 0) {
        return 0;
    }
    int index = 0;
    int length = 0;
    int splits = 0;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == del) {
            if (size == 0) {
            return -1;
        }
            arr[splits] = str.substr(index, length);
            index = i + 1;
            length = 0;
            size--;
            splits++;
        } else {
            length++;
        }
    }
    if (str[str.length() - 1 ] != del) {
        if (size == 0) {
            return -1;
        }
        arr[splits] = str.substr(index, length);
        splits++;
    }
    return splits;
}