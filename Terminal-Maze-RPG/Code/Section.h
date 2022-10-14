#ifndef SECTION_H
#define SECTION_H

using namespace std;

class Section {

    public:
        Section();
        Section(int roomType, int healthPools, bool crystal, int randCounter);
        bool getFog(int x, int y);
        int getTile(int x, int y);
        void setFog(int x, int y, bool value);
        void setTile(int x, int y, int value);
        int getResurrectionStone(int index);

    private:
        int resurrectionStone[2];
        bool fogMap[30][30];
        int map[30][30];

};

#endif