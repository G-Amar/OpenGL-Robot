/*******************************************************************
		   Hierarchical Multi-Part Model Example
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "QuadMesh.h"

const float PI = 3.142857;

const int vWidth = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotBodySize = 2.0;
float cannonLength = 0.5 * robotBodySize;
float cannonWidth = 0.2 * robotBodySize;
float notchSize = 0.8 * cannonWidth;
float notchLength = 0.5 * cannonLength;
float hipRad = 0.5*robotBodySize;
float hipLength = 0.5 * robotBodySize;
float upperLegLength = robotBodySize;
float upperLegHeight = 0.2 * robotBodySize;
float upperLegWidth = 0.3 * robotBodySize;
float lowerLegLength = 1.2 * upperLegLength;
float lowerLegHeight = upperLegHeight;
float lowerLegWidth = upperLegWidth;
float footLength = robotBodySize;
float footHeight = 0.5*robotBodySize;
float footDepth = robotBodySize;
float shoulderRad = hipRad;
float shoulderLength = 2.0 * hipLength;
float upperArmLength = 1.3*upperLegLength;
float upperArmHeight = upperLegHeight;
float upperArmWidth = upperLegWidth;
float armGunLength = 1.2 * upperArmLength;
float armGunRad = 0.5*upperArmWidth;

// Control Robot body rotation
float bodyAngle = 0.0;

// Control cannon rotation
float cannonAngle = 0.0;

// right foot
float rightHipAngle = 0.0;
float rightKneeAngle = 0.0;

// left foot
float leftHipAngle = 0.0;
float leftKneeAngle = 0.0;

float* currentRotation = NULL;

//left arm
float leftShoulderAngle = 0.0;
float leftElbowAngle = 0.0;

//right arm
float rightShoulderAngle = 0.0;
float rightElbowAngle = 0.0;


//rotates whole robot
float robotSpin = 0.0; // horzontal
float verticalSpin = 0.0;  // vertical

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat robotLeg_mat_ambient[] = { 0.25f,0.25f,0.25f,1.0f };
GLfloat robotLeg_mat_specular[] = { 0.7746f,0.7746f,0.7746f,1.0f };
GLfloat robotLeg_mat_diffuse[] = { 0.05f,0.05f,0.05f,1.0f };
GLfloat robotLeg_mat_shininess[] = { 100.0F };


GLfloat robotBody_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotBody_mat_diffuse[] = { 0.9f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// Mouse button
int currentButton;

// A flat open mesh
QuadMesh* groundMesh = NULL;

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
	VECTOR3D min;
	VECTOR3D max;
} BBox;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void cannonAnimation(int param);
void stepAnimation(int param);
void armAnimation(int param);
void drawRobot();
void drawBody();
void drawCannon();
void drawLeftLeg();
void drawRightLeg();
void drawLeftArm();
void drawRightArm();


//void drawLowerBody();

int main(int argc, char** argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("3D Hierarchical Example");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Other initializatuion
	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-16.0f, 0.0f, 16.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 32.0);
	groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const GLfloat identity[] = {1.0, 0.0, 0.0, 0.0,
								0.0, 1.0, 0.0, 0.0,
								0.0, 0.0, 1.0, 0.0,
								0.0, 0.0, 0.0, 1.0 };

	glLoadMatrixf(identity);
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();

	// Draw ground
	glPushMatrix();
	const GLfloat T1[] = {	1.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, 0.0,
							0.0, 0.0, 1.0, 0.0,
							0.0, -10.0, 0.0, 1.0 };
		glMultMatrixf(T1);
		groundMesh->DrawMesh(meshSize);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{

	
	glPushMatrix();
		
		//allows for rotating entire model horizontally (y-axis)
		glRotatef(robotSpin, 0.0, 1.0, 0.0);

		//allows for rotating entire model verticaly (x-axis)
		glRotatef(verticalSpin, 1.0, 0.0, 0.0);



		//CTM = IV*R_y*R_x
		glPushMatrix();
			
			// spin body and cannon. 
			glRotatef(bodyAngle, 1.0, 0.0, 0.0);

			//CTM = I * V * R_y * R_x * R_x(bodyAngle)
			drawBody();
			drawCannon();

		glPopMatrix();
	
		drawLeftLeg();
		drawRightLeg();
		drawLeftArm();
		drawRightArm();

	glPopMatrix();

}


void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	//CTM = I*V* R_y * R_x * R_x(bodyAngle)
	glPushMatrix();
		//CTM = I*V* R_y * R_x * R_x(bodyAngle)*S
		glScalef(robotBodySize, robotBodySize, robotBodySize);
		gluSphere(gluNewQuadric(), 1.0, 100, 100);
	glPopMatrix();
}


void drawCannon()
{
	// Set robot material properties per body part. Can have seperate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = I*V* R_y * R_x * R_x(bodyAngle)
	glPushMatrix();

		// Position cannon with respect to parent (body), place it slightly within the body
	
		glTranslatef(0.0, 0.0, (robotBodySize - 0.25 * cannonLength));

		//rotate cannon and notch on z-axis
		glRotatef(cannonAngle, 0.0, 0.0, 1.0);

		//CTM = I * V * R_y * R_x * R_x(bodyAngle) * T1 * R1
		glPushMatrix();
			// Build cannon
			glScalef(cannonWidth, cannonWidth, cannonLength);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 50, 50);

		glPopMatrix();

		// position notch above cylinder
		glTranslatef(0.0, (cannonWidth + 0.5 * notchSize), (cannonLength - 0.5 * notchLength));

		glPushMatrix();
			glScalef(notchSize, notchSize, notchLength);
			glutSolidCube(1.0);
		glPopMatrix();

	glPopMatrix();

	
}

void drawLeftLeg()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV* R_y * R_x 
	glPushMatrix();

		// Position hip with respect to parent body
		glTranslatef(robotBodySize, (-1.5 * hipRad), 0.0);

		glRotatef(90,0.0,1.0,0.0);
		
		// rotate hip and sub-parts (legs etc)
		glRotatef(leftHipAngle, 0.0, 0.0, 1.0);


		// build hip
		glPushMatrix();
			glScalef(hipRad,hipRad,hipLength);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 100, 100);
		glPopMatrix();

		//rotate 45 degrees about the hip joint
		glRotatef(45, 0.0, 0.0, 1.0);

		//position upperleg w/ resp. to hip
		glTranslatef(0.0, -(hipRad + 0.5 * upperLegLength), 0.5 * hipLength);

		//place upperleg on hip
		glPushMatrix();
			glScalef(upperLegHeight, upperLegLength, upperLegWidth);
			glutSolidCube(1.0);
		glPopMatrix();

		glTranslatef(0.0, -0.5 * upperLegLength, 0.0);
		
		//rotates will occur at the knee joint now
		glRotatef(leftKneeAngle, 0.0, 0.0, 1.0);

		//translate first to change pivot point to knee
		glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);

		glPushMatrix();
			glScalef(lowerLegLength, lowerLegHeight, lowerLegWidth);
			glutSolidCube(1.0);
		glPopMatrix();

		//position foot w/ resp. to lower leg
		glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);

		//rotate foot so that it is flat
		glRotatef(-45, 0.0, 0.0, 1.0);


		glPushMatrix();
			glScalef(footLength, footHeight, footDepth);
			glutSolidCube(1.0);
		glPopMatrix();


	glPopMatrix();
}

void drawRightLeg()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV* R_y * R_x 
	glPushMatrix();

		// Position hip with respect to parent body
		glTranslatef(-(robotBodySize + hipLength), (-1.5 * hipRad), 0.0);


		glRotatef(90, 0.0, 1.0, 0.0);

		// rotate hip and sub-parts (legs etc)
		glRotatef(rightHipAngle, 0.0, 0.0, 1.0);

		// build hip
		glPushMatrix();
			glScalef(hipRad, hipRad, hipLength);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 100, 100);
		glPopMatrix();

		//rotate 45 degrees about the hip joint
		glRotatef(45, 0.0, 0.0, 1.0);

		//position upperleg w/ resp. to hip
		glTranslatef(0.0, -(hipRad + 0.5 * upperLegLength), 0.5 * hipLength);

		//place upperleg on hip
		glPushMatrix();
			glScalef(upperLegHeight, upperLegLength, upperLegWidth);
			glutSolidCube(1.0);
		glPopMatrix();

		glTranslatef(0.0, -0.5 * upperLegLength, 0.0);

		//rotates will occur at the knee joint now
		glRotatef(rightKneeAngle, 0.0, 0.0, 1.0);

		//translate first to change pivot point to knee
		glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);
		glPushMatrix();
			glScalef(lowerLegLength, lowerLegHeight, lowerLegWidth);
			glutSolidCube(1.0);
		glPopMatrix();

		//position foot w/ resp. to lower leg
		glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);

		//rotate foot so that it is flat
		glRotatef(-45, 0.0, 0.0, 1.0);

		glPushMatrix();
			glScalef(footLength, footHeight, footDepth);
			glutSolidCube(1.0);
		glPopMatrix();


	glPopMatrix();

}

void drawLeftArm() 
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV*R_y*R_x
	glPushMatrix();

		// Position shoulder with respect to parent body
		glTranslatef(robotBodySize, 1.5 * shoulderRad, 0.0);
		glRotatef(90, 0.0, 1.0, 0.0);

		// rotate shoulder and sub-parts (arms etc)
		glRotatef(leftShoulderAngle, 0.0, 0.0, 1.0);

		// build shoulder
		glPushMatrix();
			glScalef(shoulderRad, shoulderRad, shoulderLength);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 100, 100);
		glPopMatrix();

		//rotate -45 degrees about the shoulder joint
		glRotatef(-45, 0.0, 0.0, 1.0);

		//position upperarm w/ resp. to shoulder
		glTranslatef(0.0, -(shoulderRad + 0.5 * upperArmLength), shoulderLength - 0.5 * upperArmWidth);

		//place upperArm on shoulder
		glPushMatrix();
			glScalef(upperArmHeight, upperArmLength, upperArmWidth);
			glutSolidCube(1.0);
		glPopMatrix();

		glTranslatef(0.0, -0.5 * upperArmLength, 0.0);

		//rotates will occur at the elbow joint (base of cylinder)
		glRotatef(leftElbowAngle, 0.0, 0.0, 1.0);

		glRotatef(-90, 0.0, 1.0, 0.0);

		glPushMatrix();
			glScalef(armGunRad, armGunRad, armGunLength);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 100, 100);
		glPopMatrix();

	glPopMatrix();
}

void drawRightArm()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV*R_y*R_x
	glPushMatrix();

		// Position shoulder with respect to parent body
		glTranslatef(-(robotBodySize + shoulderLength), 1.5 * shoulderRad, 0.0);
		glRotatef(90, 0.0, 1.0, 0.0);

		// rotate shoulder and sub-parts (arms etc)
		glRotatef(rightShoulderAngle, 0.0, 0.0, 1.0);

		// build shoulder
		glPushMatrix();
			glScalef(shoulderRad, shoulderRad, shoulderLength);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 100, 100);
		glPopMatrix();

		//rotate -45 degrees about the shoulder joint
		glRotatef(-45, 0.0, 0.0, 1.0);

		//position upperarm w/ resp. to shoulder
		glTranslatef(0.0, -(shoulderRad + 0.5 * upperArmLength), 0.5 * upperArmWidth);

		//place upperArm on shoulder
		glPushMatrix();
			glScalef(upperArmHeight, upperArmLength, upperArmWidth);
			glutSolidCube(1.0);
		glPopMatrix();

		glTranslatef(0.0, -0.5 * upperArmLength, 0.0);

		//rotates will occur at the elbow joint (base of cylinder)
		glRotatef(rightElbowAngle, 0.0, 0.0, 1.0);
		glRotatef(-90, 0.0, 1.0, 0.0);

		glPushMatrix();
			glScalef(armGunRad, armGunRad, armGunLength);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 100, 100);
		glPopMatrix();

	glPopMatrix();

}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool cannonRotating = false;
bool stopCannon = false;

// hip and knee rotations for animation
float hipR = 0.0, kneeR = 0.0;

float shoulderR = 0.0, elbowR = 0.0; //shoulder and elbow rotations

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	//need to add function for ARROW KEYS, this just supposed to toggle the joint
	switch (key)
	{
	case 'b':
		currentRotation = &bodyAngle;
		break;
	case 'h':
		currentRotation = &rightHipAngle;
		break;
	case 'k':
		currentRotation = &rightKneeAngle;
		break;

	case 'w':
		// reset angles and setup angle incrementers
		leftHipAngle = 0.0;
		leftKneeAngle = 0.0;
		//initialize the amount of rotation
		hipR = 1.0;
		kneeR = -1.0;
		glutTimerFunc(10, stepAnimation, 0);
		break;
	case 'W':
		leftHipAngle = 0.0;
		leftKneeAngle = 0.0;
		break;

	case 'a':
		rightShoulderAngle = 0.0;
		rightElbowAngle = 0.0;
		//initialize the amount of rotation
		shoulderR = -0.75;
		elbowR = 1.5;
		glutTimerFunc(10, armAnimation, 0);
		break;
	case 'A':
		rightShoulderAngle = 0.0;
		rightElbowAngle = 0.0;
		break;

	case 'q':
	case 'Q':
		exit(0);
		break;

	case 'c':
		stopCannon = false;
		if (!cannonRotating)
		{
			cannonRotating = true;
			// make cannon spin
			glutTimerFunc(10, cannonAnimation, 0);
		}
			break;
	case 'C':
		stopCannon = true;
		break;

	//Spins whole robot
	case 's':
		robotSpin += 2.0;
		if (robotSpin > 360.0)
			robotSpin -= 360.0;
		break;
	case 'S':
		robotSpin -= 2.0;
		if (robotSpin < 0.0)
			robotSpin += 360.0;
		break;
	case 'v':
		verticalSpin += 2.0;
		if (verticalSpin > 360.0)
			verticalSpin -= 360.0;
		break;
	case 'V':
		verticalSpin -= 2.0;
		if (verticalSpin < 0.0)
			verticalSpin += 360.0;
		break;
		
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

void cannonAnimation(int)
{
	if (!stopCannon)
	{
		cannonAngle += 1.0;
		glutPostRedisplay();
		glutTimerFunc(10, cannonAnimation, 0);
	}
	else
	{ 
		//cannon no longer rotating
		cannonRotating = false;
	}
	
}

void stepAnimation(int)
{
	//starts at 0, increments till >40
	//decrements till <-10
	//stops when <-20
	if (leftHipAngle >= 40.0)
	{
		hipR = -1.5;
		kneeR = 0.25;
	}
	else if (leftHipAngle <= -20.0)
	{
		return;
	}
	else if (leftHipAngle <= -10.0)
	{	
		hipR = -0.5;
		kneeR = 2.5;
	}
	
	leftHipAngle += hipR;
	leftKneeAngle += kneeR;
	glutPostRedisplay();
	glutTimerFunc(10, stepAnimation, 0);
}

void armAnimation(int)
{
	//starts at 0, decrement till < -45
	//return when back to starting i.e. >0
	if (rightShoulderAngle <= -45.0)
	{
		shoulderR = 0.75;
		elbowR = -1.5;
	}
	else if (rightShoulderAngle > 0.0)
	{
		return;
	}

	rightShoulderAngle += shoulderR;
	rightElbowAngle += elbowR;
	glutPostRedisplay();
	glutTimerFunc(10, armAnimation, 0);
}


// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	switch (key) 
	{
	case GLUT_KEY_LEFT:
		if (!currentRotation == NULL) {
			*currentRotation -= 2.0;
			if (*currentRotation < 0.0)
				*currentRotation += 360.0;
		}
		break;

	case GLUT_KEY_RIGHT:
		if (!currentRotation == NULL) {
			*currentRotation += 2.0;
			if (*currentRotation > 360.0)
				*currentRotation -= 360.0;
		}
		break;
	}
	/*
	// Help key
	if (key == GLUT_KEY_F1)
	{

	}
	// Do transformations with arrow keys
	//else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	//{
	//}
	*/

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

