#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif
// Default resolution
// For Retina displays compile with -DRES=2
#ifndef RES
#define RES 1
#endif

//  Globals 
int th=0;                    // Azimuth of view angle
int ph=15;                   // Elevation of view angle
int mode=2;                  //  Projection mode
int lighting = 0;            // Lighting Mode
double dim=150;              // Scene size
const double rDecrease = 3;  // Each sub-branch has 1/rDecrease the radius or 1/rDecrease^2 the area of the master branch
const double hDecrease = 2;  // Each sub-branch has 1/hDecrease the height of the master branch
const int initN = 4;         // How many layers the tree has
#define leafNum 6561         // 9^n
#define trunkNum 820         // Closed form formula for geometric series: (1-9^n)/(1-9)
#define forestSize 100       // Number of trees
bool init = false;           // For initializing forest only once
int fov=55;                  // Field of view (for perspective)
double asp=1;                // Aspect ratio
float *woodVertices;         // Global pointer to array of wood polygons
float *leafVertices;         // Global pointer to array of leaf polygons
float *groundVertices;       // Global pointer to array of ground polygons
int woodIndex = 0;           // Index of array of wood polygons
int leafIndex = 0;           // Index of array of leaf polygons
int groundIndex = 0;         // Index of array of ground polygons
static unsigned int vbo[3];  // Array holding the names of the three vbos
float distance = 100;        // Radius of light circle
float zh = 0;                // Time step for light circle
float ylight = 5;            // Height of light circle

// User Perspective
struct user
{
   double Ex,Ey,Ez;
   double dx,dy,dz;
} player1;

// Each tree consists of an x,y,z position,
// an array of trunks, and an array of leaves
// This is the vector representation of the tree
struct Tree
{
   // Position of center of base of tree
   double x,y,z;

   // Current array index, used when generating tree
   int leafPos,trunkPos;

   //Theta stuff
   int thetaStep, thetaOffset;
   
   // Each leaf consists of a cone (aka the stem) and the leaf
   struct Leaf
   {
      // Center position of base of cone
      double x,y,z;

      // Unit vector in "up" direction
      double ux,uy,uz;

      double dx,dy,dz;

      // radius and height of cone
      double r,h;
   }
   leaves[leafNum];

   // Each trunk is a hollow "tube" with a larger radius at its base than its top
   struct Trunk
   {
      // Center postion of base of trunk
      double x,y,z;

      // Unit vector in "up" direction
      double ux,uy,uz;

      double dx,dy,dz;

      // Radius at base and top of trunk, and height
      double r1,r2,h;

      int n;
   }
   trunks[trunkNum];
}
trees[forestSize];

//  Cosine and Sine in degrees
#define Cos(x) (cos((x)*3.14159265/180))
#define Sin(x) (sin((x)*3.14159265/180))


//Check for OpenGL errors
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}


// Print message to stderr and exit
void Fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}

// Convenience routine to output raster text
// Use VARARGS to make this more flexible
#define LEN 8192  // Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}


// projections
static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective projection third person
   if (mode == 1)
      gluPerspective(fov,asp,dim/4,4*dim);
   // Perspective projection first person
   else if (mode == 2)
      gluPerspective(fov,asp,dim/64,dim);
   //  Orthogonal projection
   else
      glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

// These next 3 functions are the x1,y1,z1 output of the matrix
// multiplication when you want to rotate some vector (x,y,z)
// around the unit vector (X,Y,Z)
double rotateX(double x,double y, double z,
               double X,double Y,double Z,
               double theta)
{
   return
   x*(1+(1-Cos(theta))*(X*X-1)) +
   y*(-1*Z*Sin(theta)+(1-Cos(theta))*X*Y) +
   z*(Y*Sin(theta)+(1-Cos(theta))*X*Z);
}
double rotateY(double x,double y, double z,
               double X,double Y,double Z,
               double theta)
{
   return
   x*(Z*Sin(theta)+(1-Cos(theta))*X*Y) +
   y*(1+(1-Cos(theta))*(Y*Y-1)) +
   z*(-1*X*Sin(theta)+(1-Cos(theta))*Y*Z);
}
double rotateZ(double x,double y, double z,
               double X,double Y,double Z,
               double theta)
{
   return
   x*(-1*Y*Sin(theta)+(1-Cos(theta))*X*Z) +
   y*(X*Sin(theta)+(1-Cos(theta))*Y*Z) +
   z*(1+(1-Cos(theta))*(Z*Z-1));
}

// Adds trunk to end of trunk array
void addTrunk(double x,double y,double z,
               double ux,double uy,double uz,
               double dx,double dy,double dz,
               double r1,double r2, double h,
               int n, int treeNum)
{
   int position = trees[treeNum].trunkPos;
   (trees[treeNum].trunks)[position].x = x;
   (trees[treeNum].trunks)[position].y = y;
   (trees[treeNum].trunks)[position].z = z;
   (trees[treeNum].trunks)[position].ux = ux;
   (trees[treeNum].trunks)[position].uy = uy;
   (trees[treeNum].trunks)[position].uz = uz;
   (trees[treeNum].trunks)[position].dx = dx;
   (trees[treeNum].trunks)[position].dy = dy;
   (trees[treeNum].trunks)[position].dz = dz;
   (trees[treeNum].trunks)[position].r1 = r1;
   (trees[treeNum].trunks)[position].r2 = r2;
   (trees[treeNum].trunks)[position].h = h;
   (trees[treeNum].trunks)[position].n = n;
   trees[treeNum].trunkPos += 1;
}

// Adds leaf to end of leaf array
void addLeaf(double x,double y,double z,
               double ux,double uy,double uz,
               double dx,double dy,double dz,
               double r, double h,int treeNum)
{
   int position = trees[treeNum].leafPos;
   (trees[treeNum].leaves)[position].x = x;
   (trees[treeNum].leaves)[position].y = y;
   (trees[treeNum].leaves)[position].z = z;
   (trees[treeNum].leaves)[position].ux = ux;
   (trees[treeNum].leaves)[position].uy = uy;
   (trees[treeNum].leaves)[position].uz = uz;
   (trees[treeNum].leaves)[position].dx = dx;
   (trees[treeNum].leaves)[position].dy = dy;
   (trees[treeNum].leaves)[position].dz = dz;
   (trees[treeNum].leaves)[position].r = r;
   (trees[treeNum].leaves)[position].h = h;
   trees[treeNum].leafPos += 1;
}

// Recursive function to generate the tree
void generateTreeRecursive(double x,double y,double z,
                           double ux,double uy,double uz,
                           double r,double h,
                           int n, int treeNum)
{
   // A vector orthogonal to the up vector
   // Created from the cross product of the up vector and (1,0,0)
   double dx = 0;
   double dy = -1*uz;
   double dz = uy;

   // Makes orthogonal vector a unit vector
   double unitVectorizer = sqrt(uz*uz+uy*uy);
   dy = dy / unitVectorizer;
   dz = dz / unitVectorizer;

   // Base case: adds a leaf
   if (n == 0)
   {
      addLeaf(x,y,z,ux,uy,uz,dx,dy,dz,r,h,treeNum);
   }

   else
   {
      // Main trunk
      addTrunk(x,y,z,ux,uy,uz,dx,dy,dz,r,r/rDecrease,h,n,treeNum);

      // Sub trunk directly above main trunk
      generateTreeRecursive(x+h*ux,y+h*uy,z+h*uz,ux,uy,uz,r/rDecrease,h/hDecrease,n-1,treeNum);

      // Generate 8 side trunks
      for (int i = 0; i < 8; i++)
      {
         // Starts adding side trunks halfway up the tree
         double new_x = x + ((h/2)+(h/16*i))*ux;
         double new_y = y + ((h/2)+(h/16*i))*uy;
         double new_z = z + ((h/2)+(h/16*i))*uz;

         // A normal vector orthogonal to the up vector
         // Created from the cross product of the up vector and (1,0,0)
         double nx = 0;
         double ny = -1*uz;
         double nz = uy;

         // Makes normal vector a unit vector
         double unitVectorizer = sqrt(nx*nx+ny*ny+nz*nz);
         nx = nx / unitVectorizer;
         ny = ny / unitVectorizer;
         nz = nz / unitVectorizer;

         // A unit binormal vector from the cross product
         // of the normal vector and the up vector
         double bx = ny*uz - nz*uy;
         double by = nz*ux;
         double bz = -1*ny*uz;

         // Although you shouldn't have to normalize the binormal vector
         // since it is the cross product of two unit vectors,
         // after a few recursive calls, floating point errors add up
         // thus the binormal is renormalized
         unitVectorizer = sqrt(bx*bx+by*by+bz*bz);
         bx = bx / unitVectorizer;
         by = by / unitVectorizer;
         bz = bz / unitVectorizer;

         // Theta and alpha for rotations
         int theta = (i*trees[treeNum].thetaStep+trees[treeNum].thetaOffset)%360;
         int alpha = 30;


         //rotate normal alpha degrees around binormal
         double temp_ux = rotateX(nx,ny,nz,bx,by,bz,alpha);
         double temp_uy = rotateY(nx,ny,nz,bx,by,bz,alpha);
         double temp_uz = rotateZ(nx,ny,nz,bx,by,bz,alpha);

         //rotate normal theta degrees around (ux,uy,uz)
         double new_ux = rotateX(temp_ux,temp_uy,temp_uz,ux,uy,uz,theta);
         double new_uy = rotateY(temp_ux,temp_uy,temp_uz,ux,uy,uz,theta);
         double new_uz = rotateZ(temp_ux,temp_uy,temp_uz,ux,uy,uz,theta);

         
         //call recursive function with new x,y,z and up vector
         generateTreeRecursive(new_x,new_y,new_z,new_ux,new_uy,new_uz,r/rDecrease,h/hDecrease,n-1,treeNum);
      }

      
   }
}


// The find normal functions take three points in counterclockwise
// order and return a unit normal vector of the triangle
double findNormalX(double x1,double y1,double z1,
                   double x2,double y2,double z2,
                   double x3,double y3,double z3)
{
   double a1 = x2-x1;
   double a2 = y2-y1;
   double a3 = z2-z1;

   double b1 = x3-x1;
   double b2 = y3-y1;
   double b3 = z3-z1;

   double nx = a2*b3-a3*b2;
   double ny = a3*b1-a1*b3;
   double nz = a1*b2-a2*b1;

   double magnitude = sqrt(nx*nx+ny*ny+nz*nz);

   return -1*nx/magnitude;
}
double findNormalY(double x1,double y1,double z1,
                   double x2,double y2,double z2,
                   double x3,double y3,double z3)
{
   double a1 = x2-x1;
   double a2 = y2-y1;
   double a3 = z2-z1;

   double b1 = x3-x1;
   double b2 = y3-y1;
   double b3 = z3-z1;

   double nx = a2*b3-a3*b2;
   double ny = a3*b1-a1*b3;
   double nz = a1*b2-a2*b1;

   double magnitude = sqrt(nx*nx+ny*ny+nz*nz);

   return -1*ny/magnitude;
}
double findNormalZ(double x1,double y1,double z1,
                   double x2,double y2,double z2,
                   double x3,double y3,double z3)
{
   double a1 = x2-x1;
   double a2 = y2-y1;
   double a3 = z2-z1;

   double b1 = x3-x1;
   double b2 = y3-y1;
   double b3 = z3-z1;

   double nx = a2*b3-a3*b2;
   double ny = a3*b1-a1*b3;
   double nz = a1*b2-a2*b1;

   double magnitude = sqrt(nx*nx+ny*ny+nz*nz);

   return -1*nz/magnitude;
}

// Takes a position vector, an up vector, a side vector, and trunk info
// And adds the vertices to a global array
void addVerticesTrunk(double x,double y,double z,
               double ux,double uy,double uz,
               double dx,double dy,double dz,
               double r1,double r2,double h,
               int n)
{

   // Unit vector along "x-axis"
   double D0 = sqrt(dx*dx+dy*dy+dz*dz);
   double X0 = dx/D0;
   double Y0 = dy/D0;
   double Z0 = dz/D0;

   // Unit vector along "y-axis"
   double D1 = sqrt(ux*ux+uy*uy+uz*uz);
   double X1 = ux/D1;
   double Y1 = uy/D1;
   double Z1 = uz/D1;

   // Unit vector along "z-axis"
   double X2 = Y0*Z1-Y1*Z0;
   double Y2 = Z0*X1-Z1*X0;
   double Z2 = X0*Y1-X1*Y0;

   // Vertices in lower and upper ring of trunk
   double lowerRing[80][9];
   double upperRing[40][9];
   int lowerNum = 5*pow(2,n);
   int upperNum = lowerNum/2;

   // Calculates vertices using orthonormal basis
   for (int i = 0; i < lowerNum; i++)
   {
      double theta = i*360.0/lowerNum;
      lowerRing[i][0] = (X0*Cos(theta)+X2*Sin(theta))*r2+x+h*X1;
      lowerRing[i][1] = (Y0*Cos(theta)+Y2*Sin(theta))*r2+y+h*Y1;
      lowerRing[i][2] = (Z0*Cos(theta)+Z2*Sin(theta))*r2+z+h*Z1;

      lowerRing[i][3] = (X0*Cos(theta)+X2*Sin(theta))*r1+x;
      lowerRing[i][4] = (Y0*Cos(theta)+Y2*Sin(theta))*r1+y;
      lowerRing[i][5] = (Z0*Cos(theta)+Z2*Sin(theta))*r1+z;

      double tangentx = (X0*-1*Sin(theta)+X2*Cos(theta))*r1;
      double tangenty = (Y0*-1*Sin(theta)+Y2*Cos(theta))*r1;
      double tangentz = (Z0*-1*Sin(theta)+Z2*Cos(theta))*r1;

      lowerRing[i][6] = lowerRing[i][3] + tangentx;
      lowerRing[i][7] = lowerRing[i][4] + tangenty;
      lowerRing[i][8] = lowerRing[i][5] + tangentz;
   }
   for (int i = 0; i < upperNum; i++)
   {
      double theta = i*360.0/upperNum;
      upperRing[i][0] = (X0*Cos(theta)+X2*Sin(theta))*r2+x+h*X1;
      upperRing[i][1] = (Y0*Cos(theta)+Y2*Sin(theta))*r2+y+h*Y1;
      upperRing[i][2] = (Z0*Cos(theta)+Z2*Sin(theta))*r2+z+h*Z1;

      upperRing[i][3] = (X0*Cos(theta)+X2*Sin(theta))*r1+x;
      upperRing[i][4] = (Y0*Cos(theta)+Y2*Sin(theta))*r1+y;
      upperRing[i][5] = (Z0*Cos(theta)+Z2*Sin(theta))*r1+z;

      double tangentx = (X0*-1*Sin(theta)+X2*Cos(theta))*r1;
      double tangenty = (Y0*-1*Sin(theta)+Y2*Cos(theta))*r1;
      double tangentz = (Z0*-1*Sin(theta)+Z2*Cos(theta))*r1;

      upperRing[i][6] = upperRing[i][3] + tangentx;
      upperRing[i][7] = upperRing[i][4] + tangenty;
      upperRing[i][8] = upperRing[i][5] + tangentz;
   }

   // Takes the arrays of vertices and adds triangles to the global
   // array of wood triangles
   for (int i = 0; i < upperNum; i++)
   {
      // First Triangle
      int prevVert;
      if (i == 0) prevVert = lowerNum - 1;
      else prevVert = 2*i-1;

      woodVertices[woodIndex] = upperRing[i][0];
      woodVertices[woodIndex+1] = upperRing[i][1];
      woodVertices[woodIndex+2] = upperRing[i][2];
      woodVertices[woodIndex+3] = findNormalX(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodVertices[woodIndex+4] = findNormalY(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = lowerRing[prevVert][3];
      woodVertices[woodIndex+1] = lowerRing[prevVert][4];
      woodVertices[woodIndex+2] = lowerRing[prevVert][5];
      woodVertices[woodIndex+3] = findNormalX(
         lowerRing[prevVert][0],lowerRing[prevVert][1],lowerRing[prevVert][2],
         lowerRing[prevVert][3],lowerRing[prevVert][4],lowerRing[prevVert][5],
         lowerRing[prevVert][6],lowerRing[prevVert][7],lowerRing[prevVert][8]);
      woodVertices[woodIndex+4] = findNormalY(
         lowerRing[prevVert][0],lowerRing[prevVert][1],lowerRing[prevVert][2],
         lowerRing[prevVert][3],lowerRing[prevVert][4],lowerRing[prevVert][5],
         lowerRing[prevVert][6],lowerRing[prevVert][7],lowerRing[prevVert][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         lowerRing[prevVert][0],lowerRing[prevVert][1],lowerRing[prevVert][2],
         lowerRing[prevVert][3],lowerRing[prevVert][4],lowerRing[prevVert][5],
         lowerRing[prevVert][6],lowerRing[prevVert][7],lowerRing[prevVert][8]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = lowerRing[2*i][3];
      woodVertices[woodIndex+1] = lowerRing[2*i][4];
      woodVertices[woodIndex+2] = lowerRing[2*i][5];
      woodVertices[woodIndex+3] = findNormalX(
         lowerRing[2*i][0],lowerRing[2*i][1],lowerRing[2*i][2],
         lowerRing[2*i][3],lowerRing[2*i][4],lowerRing[2*i][5],
         lowerRing[2*i][6],lowerRing[2*i][7],lowerRing[2*i][8]);
      woodVertices[woodIndex+4] = findNormalY(
         lowerRing[2*i][0],lowerRing[2*i][1],lowerRing[2*i][2],
         lowerRing[2*i][3],lowerRing[2*i][4],lowerRing[2*i][5],
         lowerRing[2*i][6],lowerRing[2*i][7],lowerRing[2*i][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         lowerRing[2*i][0],lowerRing[2*i][1],lowerRing[2*i][2],
         lowerRing[2*i][3],lowerRing[2*i][4],lowerRing[2*i][5],
         lowerRing[2*i][6],lowerRing[2*i][7],lowerRing[2*i][8]);
      woodIndex = woodIndex + 6;


      // Second Triangle
      woodVertices[woodIndex] = upperRing[i][0];
      woodVertices[woodIndex+1] = upperRing[i][1];
      woodVertices[woodIndex+2] = upperRing[i][2];
      woodVertices[woodIndex+3] = findNormalX(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodVertices[woodIndex+4] = findNormalY(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = lowerRing[2*i][3];
      woodVertices[woodIndex+1] = lowerRing[2*i][4];
      woodVertices[woodIndex+2] = lowerRing[2*i][5];
      woodVertices[woodIndex+3] = findNormalX(
         lowerRing[2*i][0],lowerRing[2*i][1],lowerRing[2*i][2],
         lowerRing[2*i][3],lowerRing[2*i][4],lowerRing[2*i][5],
         lowerRing[2*i][6],lowerRing[2*i][7],lowerRing[2*i][8]);
      woodVertices[woodIndex+4] = findNormalY(
         lowerRing[2*i][0],lowerRing[2*i][1],lowerRing[2*i][2],
         lowerRing[2*i][3],lowerRing[2*i][4],lowerRing[2*i][5],
         lowerRing[2*i][6],lowerRing[2*i][7],lowerRing[2*i][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         lowerRing[2*i][0],lowerRing[2*i][1],lowerRing[2*i][2],
         lowerRing[2*i][3],lowerRing[2*i][4],lowerRing[2*i][5],
         lowerRing[2*i][6],lowerRing[2*i][7],lowerRing[2*i][8]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = lowerRing[2*i+1][3];
      woodVertices[woodIndex+1] = lowerRing[2*i+1][4];
      woodVertices[woodIndex+2] = lowerRing[2*i+1][5];
      woodVertices[woodIndex+3] = findNormalX(
         lowerRing[2*i+1][0],lowerRing[2*i+1][1],lowerRing[2*i+1][2],
         lowerRing[2*i+1][3],lowerRing[2*i+1][4],lowerRing[2*i+1][5],
         lowerRing[2*i+1][6],lowerRing[2*i+1][7],lowerRing[2*i+1][8]);
      woodVertices[woodIndex+4] = findNormalY(
         lowerRing[2*i+1][0],lowerRing[2*i+1][1],lowerRing[2*i+1][2],
         lowerRing[2*i+1][3],lowerRing[2*i+1][4],lowerRing[2*i+1][5],
         lowerRing[2*i+1][6],lowerRing[2*i+1][7],lowerRing[2*i+1][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         lowerRing[2*i+1][0],lowerRing[2*i+1][1],lowerRing[2*i+1][2],
         lowerRing[2*i+1][3],lowerRing[2*i+1][4],lowerRing[2*i+1][5],
         lowerRing[2*i+1][6],lowerRing[2*i+1][7],lowerRing[2*i+1][8]);
      woodIndex = woodIndex + 6;

      
      // Third Triangle
      woodVertices[woodIndex] = upperRing[i][0];
      woodVertices[woodIndex+1] = upperRing[i][1];
      woodVertices[woodIndex+2] = upperRing[i][2];
      woodVertices[woodIndex+3] = findNormalX(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodVertices[woodIndex+4] = findNormalY(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         upperRing[i][0],upperRing[i][1],upperRing[i][2],
         upperRing[i][3],upperRing[i][4],upperRing[i][5],
         upperRing[i][6],upperRing[i][7],upperRing[i][8]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = lowerRing[2*i+1][3];
      woodVertices[woodIndex+1] = lowerRing[2*i+1][4];
      woodVertices[woodIndex+2] = lowerRing[2*i+1][5];
      woodVertices[woodIndex+3] = findNormalX(
         lowerRing[2*i+1][0],lowerRing[2*i+1][1],lowerRing[2*i+1][2],
         lowerRing[2*i+1][3],lowerRing[2*i+1][4],lowerRing[2*i+1][5],
         lowerRing[2*i+1][6],lowerRing[2*i+1][7],lowerRing[2*i+1][8]);
      woodVertices[woodIndex+4] = findNormalY(
         lowerRing[2*i+1][0],lowerRing[2*i+1][1],lowerRing[2*i+1][2],
         lowerRing[2*i+1][3],lowerRing[2*i+1][4],lowerRing[2*i+1][5],
         lowerRing[2*i+1][6],lowerRing[2*i+1][7],lowerRing[2*i+1][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         lowerRing[2*i+1][0],lowerRing[2*i+1][1],lowerRing[2*i+1][2],
         lowerRing[2*i+1][3],lowerRing[2*i+1][4],lowerRing[2*i+1][5],
         lowerRing[2*i+1][6],lowerRing[2*i+1][7],lowerRing[2*i+1][8]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = upperRing[(i+1)%upperNum][0];
      woodVertices[woodIndex+1] = upperRing[(i+1)%upperNum][1];
      woodVertices[woodIndex+2] = upperRing[(i+1)%upperNum][2];
      woodVertices[woodIndex+3] = findNormalX(
         upperRing[(i+1)%upperNum][0],upperRing[(i+1)%upperNum][1],upperRing[(i+1)%upperNum][2],
         upperRing[(i+1)%upperNum][3],upperRing[(i+1)%upperNum][4],upperRing[(i+1)%upperNum][5],
         upperRing[(i+1)%upperNum][6],upperRing[(i+1)%upperNum][7],upperRing[(i+1)%upperNum][8]);
      woodVertices[woodIndex+4] = findNormalY(
         upperRing[(i+1)%upperNum][0],upperRing[(i+1)%upperNum][1],upperRing[(i+1)%upperNum][2],
         upperRing[(i+1)%upperNum][3],upperRing[(i+1)%upperNum][4],upperRing[(i+1)%upperNum][5],
         upperRing[(i+1)%upperNum][6],upperRing[(i+1)%upperNum][7],upperRing[(i+1)%upperNum][8]);
      woodVertices[woodIndex+5] = findNormalZ(
         upperRing[(i+1)%upperNum][0],upperRing[(i+1)%upperNum][1],upperRing[(i+1)%upperNum][2],
         upperRing[(i+1)%upperNum][3],upperRing[(i+1)%upperNum][4],upperRing[(i+1)%upperNum][5],
         upperRing[(i+1)%upperNum][6],upperRing[(i+1)%upperNum][7],lowerRing[2*i+1][8]);
      woodIndex = woodIndex + 6;
   }
}

// Takes a position vector, an up vector, a side vector, and leaf info
// And adds the vertices to a global array
void addVerticesLeaf(double x,double y,double z,
               double ux,double uy,double uz,
               double dx,double dy,double dz,
               double r1, double h)
{
   double h1 = h/5;

   // Unit vector along "x-axis"
   double D0 = sqrt(dx*dx+dy*dy+dz*dz);
   double X0 = dx/D0;
   double Y0 = dy/D0;
   double Z0 = dz/D0;

   // Unit vector along "y-axis"
   double D1 = sqrt(ux*ux+uy*uy+uz*uz);
   double X1 = ux/D1;
   double Y1 = uy/D1;
   double Z1 = uz/D1;

   // Unit vector along "z-axis"
   double X2 = Y0*Z1-Y1*Z0;
   double Y2 = Z0*X1-Z1*X0;
   double Z2 = X0*Y1-X1*Y0;

   // Calculates vertices of cone using orthonormal basis
   double pyramidPoints[6][3];
   for (int i = 0; i < 5; i++)
   {
      double theta = i*360.0/5.0;
      pyramidPoints[i][0] = (X0*Cos(theta)+X2*Sin(theta))*r1+x;
      pyramidPoints[i][1] = (Y0*Cos(theta)+Y2*Sin(theta))*r1+y;
      pyramidPoints[i][2] = (Z0*Cos(theta)+Z2*Sin(theta))*r1+z;
   }
   pyramidPoints[5][0] = h1*X1+x;
   pyramidPoints[5][1] = h1*Y1+y;
   pyramidPoints[5][2] = h1*Z1+z;

   // Adds triangles to global array of wood triangles
   for (int i = 0; i < 5; i++)
   {
      woodVertices[woodIndex] = pyramidPoints[i][0];
      woodVertices[woodIndex+1] = pyramidPoints[i][1];
      woodVertices[woodIndex+2] = pyramidPoints[i][2];
      woodVertices[woodIndex+3] = findNormalX(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodVertices[woodIndex+4] = findNormalY(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodVertices[woodIndex+5] = findNormalZ(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = pyramidPoints[(i+1)%5][0];
      woodVertices[woodIndex+1] = pyramidPoints[(i+1)%5][1];
      woodVertices[woodIndex+2] = pyramidPoints[(i+1)%5][2];
      woodVertices[woodIndex+3] = findNormalX(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodVertices[woodIndex+4] = findNormalY(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodVertices[woodIndex+5] = findNormalZ(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodIndex = woodIndex + 6;

      woodVertices[woodIndex] = pyramidPoints[5][0];
      woodVertices[woodIndex+1] = pyramidPoints[5][1];
      woodVertices[woodIndex+2] = pyramidPoints[5][2];
      woodVertices[woodIndex+3] = findNormalX(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodVertices[woodIndex+4] = findNormalY(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodVertices[woodIndex+5] = findNormalZ(pyramidPoints[i][0],pyramidPoints[i][1],pyramidPoints[i][2],pyramidPoints[(i+1)%5][0],pyramidPoints[(i+1)%5][1],pyramidPoints[(i+1)%5][2],pyramidPoints[5][0],pyramidPoints[5][1],pyramidPoints[5][2]);
      woodIndex = woodIndex + 6;
   }

   // Leaf points in standard basis
   double leafPoints[12][3] =
   {
      {0,h1,0},
      {-0.14,h1+0.08,0.07},
      {-0.2,h1+0.2,0.1},
      {-0.2,h1+0.35,0.1},
      {-0.14,h1+0.45,0.07},
      {-0.08,h1+0.5,0.04},
      {0,h1+0.55,0},
      {0.08,h1+0.5,0.04},
      {0.14,h1+0.45,0.07},
      {0.2,h1+0.35,0.1},
      {0.2,h1+0.2,0.1},
      {0.14,h1+0.08,0.07}
   };

   // Converts leaf points to orthonormal basis
   double leafVerts[12][3];
   double leafVertsBack[12][3];
   for (int i = 0; i < 12; i++)
   {
      leafVerts[i][0] = leafPoints[i][0]*X0+leafPoints[i][1]*X1+leafPoints[i][2]*X2+x-0.001*X2;
      leafVerts[i][1] = leafPoints[i][0]*Y0+leafPoints[i][1]*Y1+leafPoints[i][2]*Y2+y-0.001*Y2;
      leafVerts[i][2] = leafPoints[i][0]*Z0+leafPoints[i][1]*Z1+leafPoints[i][2]*Z2+z-0.001*Z2;
      
      leafVertsBack[i][0] = leafVerts[i][0]+0.002*X2;
      leafVertsBack[i][1] = leafVerts[i][1]+0.002*Y2;
      leafVertsBack[i][2] = leafVerts[i][2]+0.002*Z2;
   }

   // Adds triangles to global array of leaf triangles
   for (int i = 0; i < 10; i++)
   {
      leafVertices[leafIndex] = leafVerts[0][0];
      leafVertices[leafIndex+1] = leafVerts[0][1];
      leafVertices[leafIndex+2] = leafVerts[0][2];
      leafVertices[leafIndex+3] = findNormalX(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafVertices[leafIndex+4] = findNormalY(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafVertices[leafIndex+5] = findNormalZ(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafIndex = leafIndex + 6;

      leafVertices[leafIndex] = leafVerts[i+2][0];
      leafVertices[leafIndex+1] = leafVerts[i+2][1];
      leafVertices[leafIndex+2] = leafVerts[i+2][2];
      leafVertices[leafIndex+3] = findNormalX(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafVertices[leafIndex+4] = findNormalY(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafVertices[leafIndex+5] = findNormalZ(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafIndex = leafIndex + 6;

      leafVertices[leafIndex] = leafVerts[i+1][0];
      leafVertices[leafIndex+1] = leafVerts[i+1][1];
      leafVertices[leafIndex+2] = leafVerts[i+1][2];
      leafVertices[leafIndex+3] = findNormalX(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafVertices[leafIndex+4] = findNormalY(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafVertices[leafIndex+5] = findNormalZ(leafVerts[0][0],leafVerts[0][1],leafVerts[0][2],leafVerts[i+2][0],leafVerts[i+2][1],leafVerts[i+2][2],leafVerts[i+1][0],leafVerts[i+1][1],leafVerts[i+1][2]);
      leafIndex = leafIndex + 6;
   }
   for (int i = 0; i < 10; i++)
   {
      leafVertices[leafIndex] = leafVertsBack[0][0];
      leafVertices[leafIndex+1] = leafVertsBack[0][1];
      leafVertices[leafIndex+2] = leafVertsBack[0][2];
      leafVertices[leafIndex+3] = -1*findNormalX(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafVertices[leafIndex+4] = -1*findNormalY(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafVertices[leafIndex+5] = -1*findNormalZ(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafIndex = leafIndex + 6;

      leafVertices[leafIndex] = leafVertsBack[i+2][0];
      leafVertices[leafIndex+1] = leafVertsBack[i+2][1];
      leafVertices[leafIndex+2] = leafVertsBack[i+2][2];
      leafVertices[leafIndex+3] = -1*findNormalX(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafVertices[leafIndex+4] = -1*findNormalY(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafVertices[leafIndex+5] = -1*findNormalZ(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafIndex = leafIndex + 6;

      leafVertices[leafIndex] = leafVertsBack[i+1][0];
      leafVertices[leafIndex+1] = leafVertsBack[i+1][1];
      leafVertices[leafIndex+2] = leafVertsBack[i+1][2];
      leafVertices[leafIndex+3] = -1*findNormalX(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafVertices[leafIndex+4] = -1*findNormalY(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafVertices[leafIndex+5] = -1*findNormalZ(leafVertsBack[0][0],leafVertsBack[0][1],leafVertsBack[0][2],leafVertsBack[i+2][0],leafVertsBack[i+2][1],leafVertsBack[i+2][2],leafVertsBack[i+1][0],leafVertsBack[i+1][1],leafVertsBack[i+1][2]);
      leafIndex = leafIndex + 6;
   }
}

// Takes a tree in vector form and adds triangles to
// the appropriate arrays
void addVerticesTree(int treeNum)
{
   // Adds the trunks
   for (int i = 0; i < trunkNum; i++)
   {
      addVerticesTrunk(
         (trees[treeNum].trunks)[i].x,
         (trees[treeNum].trunks)[i].y,
         (trees[treeNum].trunks)[i].z,
         (trees[treeNum].trunks)[i].ux,
         (trees[treeNum].trunks)[i].uy,
         (trees[treeNum].trunks)[i].uz,
         (trees[treeNum].trunks)[i].dx,
         (trees[treeNum].trunks)[i].dy,
         (trees[treeNum].trunks)[i].dz,
         (trees[treeNum].trunks)[i].r1,
         (trees[treeNum].trunks)[i].r2,
         (trees[treeNum].trunks)[i].h,
         (trees[treeNum].trunks)[i].n
      );
   }

   // Adds the leaves
   {
      for (int i = 0; i < leafNum; i++)
      {
         addVerticesLeaf(
            (trees[treeNum].leaves)[i].x,
            (trees[treeNum].leaves)[i].y,
            (trees[treeNum].leaves)[i].z,
            (trees[treeNum].leaves)[i].ux,
            (trees[treeNum].leaves)[i].uy,
            (trees[treeNum].leaves)[i].uz,
            (trees[treeNum].leaves)[i].dx,
            (trees[treeNum].leaves)[i].dy,
            (trees[treeNum].leaves)[i].dz,
            (trees[treeNum].leaves)[i].r,
            (trees[treeNum].leaves)[i].h
         );
      }
   }
}

// Adds triangles to the ground triangles array
void addVerticesGround()
{
   int tileNum = 22500;
   double groundPoints[5][2];
   for (int i = 0; i < tileNum; i++)
   {
      groundPoints[4][0] = i/150 - 74.5;
      groundPoints[4][1] = i%150 - 74.5;

      groundPoints[0][0] = i/150 - 75;
      groundPoints[0][1] = i%150 - 75;

      groundPoints[1][0] = i/150 - 75;
      groundPoints[1][1] = i%150 - 74;

      groundPoints[2][0] = i/150 - 74;
      groundPoints[2][1] = i%150 - 74;

      groundPoints[3][0] = i/150 - 74;
      groundPoints[3][1] = i%150 - 75;

      for (int j = 0; j < 4; j++)
      {
         groundVertices[groundIndex] = groundPoints[4][0];
         groundVertices[groundIndex+1] = 0;
         groundVertices[groundIndex+2] = groundPoints[4][1];
         groundIndex = groundIndex + 3;

         groundVertices[groundIndex] = groundPoints[j][0];
         groundVertices[groundIndex+1] = 0;
         groundVertices[groundIndex+2] = groundPoints[j][1];
         groundIndex = groundIndex + 3;

         groundVertices[groundIndex] = groundPoints[(j+1)%4][0];
         groundVertices[groundIndex+1] = 0;
         groundVertices[groundIndex+2] = groundPoints[(j+1)%4][1];
         groundIndex = groundIndex + 3;
      }
   }
}

// Called only once
// Initializes scene and randomly generates a forest
void initForest()
{
   // Temporarily store triangles in arrays before transferring to vbo
   woodVertices = (float*)malloc(337176000);
   leafVertices = (float*)malloc(944784000);
   groundVertices = (float*)malloc(3240000);

   // Initial first person coordinates
   player1.dx = 1;
   player1.dy = 0;
   player1.dz = 0;
   player1.Ex = -5;
   player1.Ey = 2;
   player1.Ez = 0;

   // Randomly generates 100 tree positions within a 150x150 square
   double xyzpositions[forestSize][2];
   for (int i = 0; i < forestSize; i++)
   {
      double xpos = rand()%146 - 72;
      double zpos = rand()%146 - 72;
      bool valid = false;
      while (!valid)
      {
         valid = true;
         for (int j = 0; j < i; j++)
         {
            if (sqrt((xyzpositions[j][0]-xpos)*(xyzpositions[j][0]-xpos)
                +(xyzpositions[j][1]-zpos)*(xyzpositions[j][1]-zpos))
                < 10)
            {
               valid = false;
               xpos = rand()%136 - 68;
               zpos = rand()%136 - 68;
               break;
            }
         }
      }
      xyzpositions[i][0] = xpos;
      xyzpositions[i][1] = zpos;
   }

   // Randomly generates tree in given position
   for (int i = 0; i < forestSize; i++)
   {
      trees[i].x = xyzpositions[i][0];
      trees[i].y = 0;
      trees[i].z = xyzpositions[i][1];
      trees[i].leafPos = 0;
      trees[i].trunkPos = 0;
      double initH = 10 + (rand()%10);
      double initR = initH/15.0;
      double thetaStep = 100 + (rand()%10);
      double thetaOffset = rand()%360;
      trees[i].thetaStep = thetaStep;
      trees[i].thetaOffset = thetaOffset;
      generateTreeRecursive(trees[i].x,trees[i].y,trees[i].z,0,1,0,initR,initH,initN,i);
   }

   // Take the forest in vector form and convert it to triangle form
   for (int i = 0; i < forestSize; i++)
   {
      addVerticesTree(i);
   }
   addVerticesGround();

   // Generates and populates 3 vbos for wood, leaves, and ground
   glGenBuffers(3, vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glBufferData(GL_ARRAY_BUFFER,3240000,groundVertices,GL_DYNAMIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
   glBufferData(GL_ARRAY_BUFFER,337176000,woodVertices,GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
   glBufferData(GL_ARRAY_BUFFER,944784000,leafVertices,GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER,0);

   // frees global arrays
   free(groundVertices);
   free(woodVertices);
   free(leafVertices);
}

// Draws forest using vbos
void drawForest()
{
   float groundBrown[]  = {123.0/256,63.0/256,0.0/256,1.0};
   float treeBrown[]  = {110.0/256,38.0/256,14.0/256,1.0};
   float leafGreen[]  = {45.0/256,90.0/256,39.0/256,1.0};
   float flashlightWhite[]  = {249.0/256,238.0/256,214.0/256,1.0};
   float shiny = 2;

   // Drawing ground
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glVertexPointer(3,GL_FLOAT,3*sizeof(float),(void*)0);
   glEnableClientState(GL_VERTEX_ARRAY);
   glNormal3f(0,1,0);
   glColor4fv(groundBrown);
   glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,groundBrown);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,flashlightWhite);
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glDrawArrays(GL_TRIANGLES,0,270000);
   glDisableClientState(GL_VERTEX_ARRAY);

   // Drawing wood
   glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
   glVertexPointer(3,GL_FLOAT,6*sizeof(float),(void*)0);
   glEnableClientState(GL_VERTEX_ARRAY);
   glNormalPointer(GL_FLOAT,6*sizeof(float),(void*)12);
   glEnableClientState(GL_NORMAL_ARRAY);
   glColor4fv(treeBrown);
   glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,treeBrown);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,flashlightWhite);
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glDrawArrays(GL_TRIANGLES,0,14049000);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);

   // Drawing leaves
   glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
   glVertexPointer(3,GL_FLOAT,6*sizeof(float),(void*)0);
   glEnableClientState(GL_VERTEX_ARRAY);
   glNormalPointer(GL_FLOAT,6*sizeof(float),(void*)12);
   glEnableClientState(GL_NORMAL_ARRAY);
   glColor4fv(leafGreen);
   glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,leafGreen);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,flashlightWhite);
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glDrawArrays(GL_TRIANGLES,0,39366000);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glBindBuffer(GL_ARRAY_BUFFER,0);
}

void display()
{
   // Generates trees on startup
   if (init == false)
   {
      init = true;
      srand(time(NULL));
      initForest();
   }

   // Clear the image
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   // Reset previous transforms
   glLoadIdentity();

   // Enable z-buffering
   glutInitDisplayMode(GLUT_DEPTH);
   glEnable(GL_DEPTH_TEST);

   // Third person perspective
   if (mode == 1)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }

   // First person perspective
   else if (mode == 2)
   {
      gluLookAt(player1.Ex,player1.Ey,player1.Ez,player1.Ex+player1.dx,player1.Ey+player1.dy,player1.Ez+player1.dz,0,1,0);
   }

   // Orthogonal
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }

   // No lighting
   if (lighting == 2)
   {
      drawForest();
   
   // Circle light
   } else if (lighting == 1)
   {
      glShadeModel(GL_SMOOTH);
      float Ambient[]   = {0,0,0,1.0};
      float Diffuse[]   = {0.4,0.4,0.4,1.0};
      float Specular[]  = {0.1,0.1,0.1,1.0};
      float Position[] = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
      glEnable(GL_NORMALIZE);
      glEnable(GL_LIGHTING);
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
      drawForest();
      glDisable(GL_LIGHTING);
      glColor3f(1,1,1);
      glBegin(GL_TRIANGLE_FAN);
      glVertex3d(Position[0],Position[1],Position[2]);
      glVertex3d(Position[0]-1,Position[1]-1,Position[2]-1);
      glVertex3d(Position[0]+1,Position[1]-1,Position[2]-1);
      glVertex3d(Position[0],Position[1]-1,Position[2]+1);
      glVertex3d(Position[0]-1,Position[1]-1,Position[2]-1);
      glEnd();   
   } else

   // Following light
   {
      glShadeModel(GL_SMOOTH);
      float Ambient[]   = {0,0,0,1.0};
      float Diffuse[]   = {0.4,0.4,0.4,1.0};
      float Specular[]  = {0.1,0.1,0.1,1.0};
      float Position[] = {player1.Ex,player1.Ey,player1.Ez,1.0};
      glEnable(GL_NORMALIZE);
      glEnable(GL_LIGHTING);
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
      drawForest();
      glDisable(GL_LIGHTING);
   }
   
   // Flush and swap
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}


void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
   {
      exit(0);
   }

   // Change circle light radius and height
   else if (ch == 'j') distance++;
   else if (ch == 'J') distance--;
   else if (ch == 'k') ylight++;
   else if (ch == 'K') ylight--;

   //  Reset view angle
   else if (ch == '0')
   {
      th = 0;
      ph = 15;
      th = 0;
      ph = 15;
      player1.dx = 1;
      player1.dy = 0;
      player1.dz = 0;
      player1.Ex = -5;
      player1.Ey = 2;
      player1.Ez = 0;
   }

   // Change projection mode
   else if (ch == 'm')
   {
      mode = (mode + 1) % 3;
      th = 0;
      ph = 15;
      player1.dx = 1;
      player1.dy = 0;
      player1.dz = 0;
      player1.Ex = -5;
      player1.Ey = 2;
      player1.Ez = 0;
   }

   // Change lighting mode
   else if (ch == 'n')
   {
      lighting = (lighting+1)%3;
   }

   // Zoom in
   else if (ch == '+')
   {
      dim = (dim == 30) ? 30 : dim - 1;
   }

   // Zoom out
   else if (ch == '-')
   {
      dim = (dim == 70) ? 70 : dim + 1;
   }

   // Move forward
   else if (ch == 'w')
   {
      player1.Ex += Cos(th);
      player1.Ez += Sin(th);
   }

   // Move backward
   else if (ch == 's')
   {
      player1.Ex -= Cos(th);
      player1.Ez -= Sin(th);
   }

   // Move left
   else if (ch == 'a')
   {
      player1.Ex += Cos(th-90);
      player1.Ez += Sin(th-90);
   }

   // Move right
   else if (ch == 'd')
   {
      player1.Ex += Cos(th+90);
      player1.Ez += Sin(th+90);
   }

   // Move up
   else if (ch == 'q')
   {
      player1.Ey += 0.75;
   }

   // Move down
   else if (ch == 'z')
   {
      player1.Ey -= 0.75;
   }

   // Tell GLUT it is necessary to redisplay the scene
   Project();
   glutPostRedisplay();
}

void special(int key,int x,int y)
{
   //  Right arrow key - increase azimuth by 5 degrees
   if (key == GLUT_KEY_RIGHT)
   {
      th += 5;
      player1.dx = Cos(th);
      player1.dz = Sin(th);
   }
   //  Left arrow key - decrease azimuth by 5 degrees
   else if (key == GLUT_KEY_LEFT)
   {
      th -= 5;
      player1.dx = Cos(th);
      player1.dz = Sin(th);
   }
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP && ph < 90)
   {
      ph += 5;
      player1.dy = Sin(ph);
   }

   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN && ph > -90)
   {
      ph -= 5;
      player1.dy = Sin(ph);
   }

   //  Keep theta angle to +/-360 degrees
   th %= 360;

   //  Tell GLUT it is necessary to redisplay the scene
   Project();
   glutPostRedisplay();
}

void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

// GLUT calls this routine when the window is resized
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Set projection
   Project();
}


// Start up GLUT and tell it what to do
int main(int argc,char* argv[])
{
  //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window , and depth
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Request 500 x 500 pixel window
   glutInitWindowSize(500,500);
   //  Create the window
   glutCreateWindow("EIRIK_SKILDHEIM_HW5");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   // Tell GLUT to call idle when nothing else to do
   glutIdleFunc(idle);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   //  Return code
   return 0;
}