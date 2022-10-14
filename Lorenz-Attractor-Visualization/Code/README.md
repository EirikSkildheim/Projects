In order to run, you must have OpenGL and GLUT installed

Type make while in the same directory as lorenz.c and use ./lorenz to run


There are two modes to the lorenz.c program:
Butterfly mode and Line mode

General Controls:

ESC : Exit program
Left/Right/Up/Down Arrow Keys : Move the scene
0 : Reset scene
r : Generate new random scene
m : Change mode


Lorenz Paramaters:

a/A : lower or raise the lorenz 's' paramater
s/S : lower or raise the lorenz 'b' paramater
d/D : lower or raise the lorenz 'r' paramater
9 : reset lorenz paramaters


Butterfly Mode:

Displays animated butterflies starting from a random position
Butterflies move using the lorenz attractor equations


Butterfly mode controls:

n : Change number (and size) of butterflies from 1-10-100-1000-10000



Line Mode:

Displays a single line from a random x,y,x starting position
Color changes from red-yellow-green-cyan-blue
