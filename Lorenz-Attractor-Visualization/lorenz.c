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
#ifndef RES
#define RES 1
#endif


//  Globals
int th=0;                         // Azimuth of view angle
int ph=0;                         // Elevation of view angle
double dim=50;                    // Scene size
double dt = 0;                    // Time in seconds since last frame
double prevTime = 0.0;            // prevTime and currTime used to calculate dt
double currTime;                  // Since GLUT only records time since launch
double bscale = 3;                // Butterfly size
bool uninit = true;               // To initialize the program
const int numPoints = 30000;      // Number of data points
int butterNum = 1;                // Number of butterflies
int mode = 1;                     // Switch between two modes
int initX;                        // Initial x for generating points
int initY;                        // Initial y for generating points
int initZ;                        // Initial z for generating points
double s  = 10;                   // ***********************
double b  = 2.6666;               // ***Lorenz Parameters***
double r  = 28;                   // ***********************
const double t = 0.001;           // Time step for line mode
const double flySpeed = 0.04;     // Speed of butterfly flight
const double rotateSpeed = 250;   // Speed of butterfly wing flap

//  Cosine and Sine in degrees
#define Cos(x) (cos((x)*3.14159265/180))
#define Sin(x) (sin((x)*3.14159265/180))


// Global array of points for line mode
struct dataPoint {
   double x;
   double y;
   double z;
} dataPoints[numPoints];

// Global array of positions for butterflies
struct butterfly {
   // Position vector
   double x,y,z;
   // Forward Vector
   double dx,dy,dz;
   // Up Vector
   double ux,uy,uz;
   // Angle of wings
   double theta;
} butterflies[10000];

//Check for OpenGL errors
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

// Generates points for line mode
void generateDataPoints(double new_x, double new_y, double new_z) {

   // First point
   dataPoints[0].x = new_x;
   dataPoints[0].y = new_y;
   dataPoints[0].z = new_z;

   // Next 29999 points
   for (int i = 1; i < numPoints; i++)
   {
      // These are the lorenz equations
      double dx = s*(dataPoints[i-1].y-dataPoints[i-1].x);
      double dy = dataPoints[i-1].x*(r-dataPoints[i-1].z)-dataPoints[i-1].y;
      double dz = dataPoints[i-1].x*dataPoints[i-1].y - b*dataPoints[i-1].z;
      dataPoints[i].x = dataPoints[i-1].x + t*dx;
      dataPoints[i].y = dataPoints[i-1].y + t*dy;
      dataPoints[i].z = dataPoints[i-1].z + t*dz;
   }
}

// Updates butterfly position and direction vectors
void updateButterfly(int i) {
   double x1,y1,z1,dx1,dy1,dz1;

   // Calculates next position and direction vector using lorenz equations
   dx1 = s*(butterflies[i].y-butterflies[i].x);
   dy1 = butterflies[i].x*(r-butterflies[i].z)-butterflies[i].y;
   dz1 = butterflies[i].x*butterflies[i].y - b*butterflies[i].z;
   x1 = butterflies[i].x + dt*flySpeed*dx1;
   y1 = butterflies[i].y + dt*flySpeed*dy1;
   z1 = butterflies[i].z + dt*flySpeed*dz1;

   // Calculates up vector using the triple cross product a x (a x b)
   // Where a is the new direction and b is the previous direction
   butterflies[i].ux = (dy1*((dx1*butterflies[i].dy)-(dy1*butterflies[i].dx)))-(dz1*((dz1*butterflies[i].dx)-(dx1*butterflies[i].dz)));
   butterflies[i].uy = (dz1*((dy1*butterflies[i].dz)-(dz1*butterflies[i].dy)))-(dx1*((dx1*butterflies[i].dy)-(dy1*butterflies[i].dx)));
   butterflies[i].uy = (dx1*((dz1*butterflies[i].dz)-(dz1*butterflies[i].dz)))-(dy1*((dy1*butterflies[i].dz)-(dz1*butterflies[i].dy)));

   // Assigns updated values
   butterflies[i].x = x1;
   butterflies[i].y = y1;
   butterflies[i].z = z1;
   butterflies[i].dx = dx1;
   butterflies[i].dy = dy1;
   butterflies[i].dz = dz1;

   // Flaps the wings by rotating
   butterflies[i].theta = butterflies[i].theta + ((dt * rotateSpeed));
   if (butterflies[i].theta > 220) {
      butterflies[i].theta -= 220;
   }
}

static void drawButterfly(double x,double y,double z,
                       double dx,double dy,double dz,
                       double ux,double uy, double uz, double theta1, int i)
{
   // Determines butterfly color based off of the postion in array
   // Cycles through colors from red-yellow-green-cyan-blue
   double num = (double)i*4/butterNum;
   float red, green, blue;
   if (num < 1)
   {
      red = 1;
      green = num;
      blue = 0;
   } else if (num < 2)
   {
      red = 2 - num;
      green = 1;
      blue = 0;
   } else if (num < 3)
   {
      red = 0;
      green = 1;
      blue = num - 2;
   } else
   {
      red = 0;
      green = 4 - num;
      blue = 1;
   }
   glColor3f(red,green,blue);

   // Convert theta to integer between -30 and 80
   int theta;
   if (theta1 < 110) {
      theta = (int)(theta1 - 30);
   } else {
      theta = (int)(190 - theta1);
   }

   // Unit vector in direction of flight
   double D0 = sqrt(dx*dx+dy*dy+dz*dz);
   double X0 = dx/D0;
   double Y0 = dy/D0;
   double Z0 = dz/D0;

   // Unit vector in "up" direction
   double D1 = sqrt(ux*ux+uy*uy+uz*uz);
   double X1 = ux/D1;
   double Y1 = uy/D1;
   double Z1 = uz/D1;

   // Cross product gives the third vector
   double X2 = Y0*Z1-Y1*Z0;
   double Y2 = Z0*X1-Z1*X0;
   double Z2 = X0*Y1-X1*Y0;

   // Rotation matrix that tranforms coordinates from standard
   // basis to a new orthonormal basis (x,y,z) -> (x1,y1,z1)
   double mat[16];
   mat[0] = X0;   mat[4] = X1;   mat[ 8] = X2;   mat[12] = 0;
   mat[1] = Y0;   mat[5] = Y1;   mat[ 9] = Y2;   mat[13] = 0;
   mat[2] = Z0;   mat[6] = Z1;   mat[10] = Z2;   mat[14] = 0;
   mat[3] =  0;   mat[7] =  0;   mat[11] =  0;   mat[15] = 1;

   // Save current transforms
   glPushMatrix();

   // Offset, scale and rotate
   glTranslated(x,y,z);
   glMultMatrixd(mat);
   glScaled(bscale,bscale,bscale);

   // Draw body of butterfly
   glBegin(GL_QUAD_STRIP);
   for (int th=0;th<=360;th+=30)
   {
      glVertex3d(-1.1,0.05*Cos(th),0.05*Sin(th));
      glVertex3d(0.2,0.05*Cos(th),0.05*Sin(th));
   }
   glEnd();

   // Draw antennas
   glLineWidth(0.5);
   glBegin(GL_LINES);
   glVertex3d(0.1, 0, 0);
   glVertex3d(0.6, 0, -0.15);
   glVertex3d(0.1, 0, 0);
   glVertex3d(0.6, 0, 0.15);
   glEnd();

   // Draw butt cover
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(-1.1, 0.0, 0.0);
   for (int th=0;th<=360;th+=30)
      glVertex3d(-1.1,0.05*Cos(th),0.05*Sin(th));
   glEnd();

   // Draw head cone
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(0.25, 0.0, 0.0);
   for (int th=0;th<=360;th+=30)
      glVertex3d(0.2,0.05*Cos(th),0.05*Sin(th));
   glEnd();

   // Draw wings
   // Extra push matrices to rotate just wings
   // Wing 1
   glPushMatrix();
   glScaled(1,1,2);
   glPushMatrix();
   glRotated(theta,1,0,0);
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(0, 0, 0);
   glVertex3d(1, 0, -0.7);
   glVertex3d(1.1, 0, -1);
   glVertex3d(0.85, 0, -1.7);
   glVertex3d(0, 0, -1.2);
   glVertex3d(-0.05, 0, -1.0);
   glVertex3d(-0.2, 0, -1.15);
   glVertex3d(-1.6, 0, -1);
   glVertex3d(-1.85, 0, -0.4);
   glVertex3d(-1.7, 0, -0.2);
   glVertex3d(-1, 0, 0);
   glEnd();
   glPopMatrix();

   // Wing 2
   glPushMatrix();
   glRotated(-1*theta,1,0,0);
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(0, 0, 0);
   glVertex3d(1, 0, 0.7);
   glVertex3d(1.1, 0, 1);
   glVertex3d(0.85, 0, 1.7);
   glVertex3d(0, 0, 1.2);
   glVertex3d(-0.05, 0, 1.0);
   glVertex3d(-0.2, 0, 1.15);
   glVertex3d(-1.6, 0, 1);
   glVertex3d(-1.85, 0, 0.4);
   glVertex3d(-1.7, 0, 0.2);
   glVertex3d(-1, 0, 0);
   glEnd();

   // Undo transformations
   glPopMatrix();
   glPopMatrix();
   glPopMatrix();
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


void display()
{
   // Clear the image
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   // Reset previous transforms
   glLoadIdentity();

   // Squashes scene along z-axis so objects don't clip when too "close"
   glScaled(1,1,0.001);

   // Set view angle
   glRotated(ph,1,0,0);
   glRotated(th,0,1,0);
   

   // Enable z-buffering
   glutInitDisplayMode(GLUT_DEPTH);
   glEnable(GL_DEPTH_TEST);

   // Runs once when program starts, populates line mode array and butterfly array
   if (uninit == true) {
      uninit = false;
      // Seeds random num generator
      srand(time(NULL));
      
      // Chooses random x,y,z between -8 and 10
      initX = (rand() % 18) - 8;
      initY = (rand() % 18) - 8;
      initZ = (rand() % 18) - 8;

      // Doesn't allow all values to be 0
      if (initX == 0 && initY == 0 && initZ == 0) {
         initX = 1;
      }

      // Populates array of points
      generateDataPoints(initX,initY,initZ);

      // Populates butterfly position array
      for (int i = 0; i < butterNum; i++) {
         double x,y,z,dx,dy,dz;

         // Chooses random value for x,y,z between -30 and 30
         x = (rand() % 60) - 30;
         y = (rand() % 60) - 30;
         z = (rand() % 60) - 30;

         // Calculates first position using lorenz equations
         dx = s*(y-x);
         dy = x*(r-z)-y;
         dz = x*y - b*z;
         x = x + 0.001*dx;
         y = y + 0.001*dy;
         z = z + 0.001*dz;

         // Assigns values
         butterflies[i].x = x;
         butterflies[i].y = y;
         butterflies[i].z = z;
         butterflies[i].dx = dx;
         butterflies[i].dy = dy;
         butterflies[i].dz = dz;

         // Intitial "up" vector is unit vector in y direction
         butterflies[i].ux = 0;
         butterflies[i].uy = 1;
         butterflies[i].uz = 0;

         // Random initial angle between -30 and 80 degrees
         butterflies[i].theta = (double)(rand() % 220);
      }
   }

   // line mode
   if (mode == 0)
   {
      // Draw multicolored line with order red-yellow-green-cyan-blue
      glLineWidth(0.75);
      for (int i = 1; i < numPoints; i++)
      {
         double num = (double)i*4/numPoints;
         float red, green, blue;
         if (num < 1)
         {
            red = 1;
            green = num;
            blue = 0;
         } else if (num < 2)
         {
            red = 2 - num;
            green = 1;
            blue = 0;
         } else if (num < 3)
         {
            red = 0;
            green = 1;
            blue = num - 2;
         } else
         {
            red = 0;
            green = 4 - num;
            blue = 1;
         }
         glColor3f(red,green,blue);
         glBegin(GL_LINES);
         glVertex3d(dataPoints[i-1].x,dataPoints[i-1].y,dataPoints[i-1].z);
         glVertex3d(dataPoints[i].x,dataPoints[i].y,dataPoints[i].z);
         glEnd();
      }
   }

   // Butterfly mode
   else if (mode == 1)
   {
      for (int i = 0; i < butterNum; i++) {
         // Updates position and direction
         updateButterfly(i);
         // Draws butterfly
         drawButterfly(butterflies[i].x,butterflies[i].y,butterflies[i].z,butterflies[i].dx,butterflies[i].dy,butterflies[i].dz,butterflies[i].ux,butterflies[i].uy,butterflies[i].uz,butterflies[i].theta, i);
      }
   }


   // Draw axes in white
   glLineWidth(2);
   glColor3f(1,1,1);
   glBegin(GL_LINES);
   glVertex3d(0,0,0);
   glVertex3d(30,0,0);
   glVertex3d(0,0,0);
   glVertex3d(0,30,0);
   glVertex3d(0,0,0);
   glVertex3d(0,0,30);
   glEnd();




   
   // Label axes
   glRasterPos3d(30,0,0);
   Print("X");
   glRasterPos3d(0,30,0);
   Print("Y");
   glRasterPos3d(0,0,30);
   Print("Z");

   // Display parameters
   glWindowPos2i(5,5);
   if (mode == 0) Print("LINE MODE  init-x: %d, init-y: %d, init-z: %d, view-angle: %d, %d, s: %f, b: %f, r: %f",initX,initY,initZ,th,ph,s,b,r);
   if (mode == 1)
   {
      char* size;
      if (butterNum == 1) size = "Huge";
      if (butterNum == 10) size = "Large";
      if (butterNum == 100) size = "Medium";
      if (butterNum == 1000) size = "Small";
      if (butterNum == 10000) size = "Tiny";
      Print("BUTTERFLY MODE  view-angle: %d, %d, Num: %d, Size: %s, s: %f, b: %f, r: %f",th,ph,butterNum,size,s,b,r);
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

   //  Reset view angle
   else if (ch == '0')
   {
      th = ph = 0;
   }
   // Generate new array of points and butterflies with new lorenz paramters
   else if (ch == 'a' || ch == 'A' || ch == 's'
            || ch == 'S' || ch == 'd' || ch == 'D'
            || ch == '9')
   {
      //  Reset lorenz paramaters
      if (ch == '9')
      {
         s  = 10;
         b  = 2.6666;
         r  = 28;
      }
      // Change lorenz paramaters
      else if (ch == 'a')
      {
         s = s * 0.90909090909;
      }
      else if (ch == 'A')
      {
         s = s * 1.1;
      }
      else if (ch == 's')
      {
         b = b * 0.90909090909;
      }
      else if (ch == 'S')
      {
         b = b * 1.1;
      }
      else if (ch == 'd')
      {
         r = r * 0.90909090909;
      }
      else if (ch == 'D')
      {
         r = r * 1.1;
      }

      // Generate new points
      
      // Line mode
      if (mode == 0) {
         initX = (rand() % 18) - 8;
         initY = (rand() % 18) - 8;
         initZ = (rand() % 18) - 8;
         if (initX == 0 && initY == 0 && initZ == 0) {
            initX = 1;
         }
         generateDataPoints(initX,initY,initZ);

      // Butterfly mode
      } else {
         // New random butterfly positions
         for (int i = 0; i < butterNum; i++) {
            double x,y,z,dx,dy,dz;

            // Random x,y,z between -30 and 30
            x = (rand() % 60) - 30;
            y = (rand() % 60) - 30;
            z = (rand() % 60) - 30;

            // Calculate first position and vector with lorenz equations
            dx = s*(y-x);
            dy = x*(r-z)-y;
            dz = x*y - b*z;
            x = x + dt*flySpeed*dx;
            y = y + dt*flySpeed*dy;
            z = z + dt*flySpeed*dz;

            // Assign values
            butterflies[i].x = x;
            butterflies[i].y = y;
            butterflies[i].z = z;
            butterflies[i].dx = dx;
            butterflies[i].dy = dy;
            butterflies[i].dz = dz;

            // Intitial "up" vector is unit vector in y direction
            butterflies[i].ux = 0;
            butterflies[i].uy = 1;
            butterflies[i].uz = 0;

            // Random initial theta
            butterflies[i].theta = (double)(rand() % 220);
         }
      }
   }

   // Generates new random array of either points or butterflies
   else if (ch == 'r')
   {

      // Line mode
      if (mode == 0) {
         initX = (rand() % 18) - 8;
         initY = (rand() % 18) - 8;
         initZ = (rand() % 18) - 8;
         if (initX == 0 && initY == 0 && initZ == 0) {
            initX = 1;
         }
         generateDataPoints(initX,initY,initZ);

      // Butterfly mode
      } else {
         // New random butterfly positions
         for (int i = 0; i < butterNum; i++) {
            double x,y,z,dx,dy,dz;

            // Random x,y,z between -30 and 30
            x = (rand() % 60) - 30;
            y = (rand() % 60) - 30;
            z = (rand() % 60) - 30;

            // Calculate first position and vector with lorenz equations
            dx = s*(y-x);
            dy = x*(r-z)-y;
            dz = x*y - b*z;
            x = x + dt*flySpeed*dx;
            y = y + dt*flySpeed*dy;
            z = z + dt*flySpeed*dz;

            // Assign values
            butterflies[i].x = x;
            butterflies[i].y = y;
            butterflies[i].z = z;
            butterflies[i].dx = dx;
            butterflies[i].dy = dy;
            butterflies[i].dz = dz;

            // Intitial "up" vector is unit vector in y direction
            butterflies[i].ux = 0;
            butterflies[i].uy = 1;
            butterflies[i].uz = 0;

            // Random initial theta
            butterflies[i].theta = (double)(rand() % 220);
         }
      }
   }
   // Change mode
   else if (ch == 'm')
   {
      mode = (mode + 1) % 2;
   }
   // Change number and size of butterflies
   else if (ch == 'n') {
      if (butterNum == 1) {butterNum = 10; bscale = 2;}
      else if (butterNum == 10) {butterNum = 100; bscale = 0.75;}
      else if (butterNum == 100) {butterNum = 1000; bscale = 0.3;}
      else if (butterNum == 1000) {butterNum = 10000; bscale = 0.1;}
      else if (butterNum == 10000) {butterNum = 1; bscale = 3;}

      // New random array of butterfly postions and vectors
      for (int i = 0; i < butterNum; i++) {
         double x,y,z,dx,dy,dz;

         x = (rand() % 60) - 30;
         y = (rand() % 60) - 30;
         z = (rand() % 60) - 30;

         dx = s*(y-x);
         dy = x*(r-z)-y;
         dz = x*y - b*z;
         x = x + dt*flySpeed*dx;
         y = y + dt*flySpeed*dy;
         z = z + dt*flySpeed*dz;

         butterflies[i].x = x;
         butterflies[i].y = y;
         butterflies[i].z = z;
         butterflies[i].dx = dx;
         butterflies[i].dy = dy;
         butterflies[i].dz = dz;
         butterflies[i].ux = 0;
         butterflies[i].uy = 1;
         butterflies[i].uz = 0;
         butterflies[i].theta = (double)(rand() % 220);
      }
   }

   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

void special(int key,int x,int y)
{
   //  Right arrow key - increase azimuth by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;

   //  Left arrow key - decrease azimuth by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;

   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
   {
      ph += 5;
   }

   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
   {
      ph -= 5;
   }

   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;

   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}


// GLUT calls this routine when the window is resized
void reshape(int width,int height)
{
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection box adjusted for the
   //  aspect ratio of the window
   double asp = (height>0) ? (double)width/height : 1;
   glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

// GLUT calls this function when there is nothing else to do
void idle()
{
   // Get the time elapsed in milliseconds since the program began and convert to seconds
   currTime = glutGet(GLUT_ELAPSED_TIME)/1000.0;

   // Calculate dt in seconds since last frame
   dt = currTime - prevTime;

   // Assign current frame time to previous time
   prevTime = currTime;

   // Redisplay the scene
   glutPostRedisplay();
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
   glutCreateWindow("Lorenz Attractor Visualization");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Tell GLUT to call "idle" when there is nothing else to do
   glutIdleFunc(idle);
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   //  Return code
   return 0;
}