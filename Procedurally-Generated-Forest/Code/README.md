## In order to run, you must have OpenGL and GLUT installed

## Type make while in the same directory as forest.c and use ./forest to run

#### This program recursively generates 100 trees and stores the resulting 17 million triangles and normals in a VBO.

#### There are 3 lighting modes:

Mode 0 has the light follow the camera as if using a latern or flashlight

Mode 1 has the light circle around the forest

Mode 2 has no lighting


#### General Controls:

ESC : Exit program

0 : Reset scene

m : change projection mode

n : change lighting mode


#### Third Person Controls:

+/- : Zoom in and out

Left/Right/Up/Down Arrows : Move the scene

#### First Person Controls:

w/a/s/d : Move the "player" on the plane

q/z : Move the "player" up and down

Left/Right/Up/Down Arrows : Look in that direction

#### Lighting Controls:

j/J : Increase/Decrease radius of light circle

k/K : Increase/Decrease height of light circle


