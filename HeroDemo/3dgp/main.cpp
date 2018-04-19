#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(x)	((x) * (float)M_PI / 180.f)

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;

// 3D Models
C3dglTerrain terrain, water;
C3dglSkyBox skybox;
C3dglModel tree1, palm1, cabin, firepit, palmbush, pier, lamp;
C3dglModel mirror;

// texture ids
GLuint idTexGrass;		// grass texture
GLuint idTexSand;		// sand texture
GLuint idTexWater;		// water texture
GLuint idTexNone;
GLuint idTexCabin;		// cabin texture
GLuint idTexPier;
GLuint idTexLamp;
GLuint idTexFirepit;
GLuint idTexPalmBush;
GLuint idTexParticle;
GLuint idTexParticleRain;
GLuint idBufferVelocity;
GLuint idBufferStartTime;
GLuint idBufferInitialPos;
GLuint idBufferVelocityRain;
GLuint idBufferStartTimeRain;
GLuint idBufferInitialPosRain;
GLuint idTexWood;
GLuint idTexCloth;

// GLSL Objects (Shader Program)
C3dglProgram ProgramBasic;
C3dglProgram ProgramWater;
C3dglProgram ProgramTerrain;
C3dglProgram ProgramParticle;
C3dglProgram ProgramParticleRain;
C3dglProgram ProgramFog;

// mirror angle
float angleFrame = 45, angleMirror = 90, deltaFrame = 0, deltaMirror = 0;

// Water specific variables
float waterLevel = 4.8f;

// Particle System Params
const float PERIOD = 0.001f;
const float PERIOD_RAIN = 0.0000075f;
const float LIFETIME = 4;
const float LIFETIME_RAIN = 5.0;
const int NPARTICLES = (int)(LIFETIME / PERIOD);
const int NPARTICLES_RAIN = (int)(LIFETIME_RAIN / PERIOD_RAIN);

// camera position (for first person type camera navigation)
float matrixView[16];		// The View Matrix
float angleTilt = 0;		// Tilt Angle
float deltaX = 0, deltaY = 0, deltaZ = 0;	// Camera movement values

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// --------------------------- SHADER INITIALIZATION ---------------------- //

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	// Initialise Shader - Basic
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramBasic.Create()) return false;
	if (!ProgramBasic.Attach(VertexShader)) return false;
	if (!ProgramBasic.Attach(FragmentShader)) return false;
	if (!ProgramBasic.Link()) return false;
	if (!ProgramBasic.Use(true)) return false;

	// Initialise Shader - Particle
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/particles.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/particles.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramParticle.Create()) return false;
	if (!ProgramParticle.Attach(VertexShader)) return false;
	if (!ProgramParticle.Attach(FragmentShader)) return false;
	if (!ProgramParticle.Link()) return false;
	if (!ProgramParticle.Use(true)) return false;

	// Initialise Shader - ParticleRain
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/particles_rain.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/particles_rain.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramParticleRain.Create()) return false;
	if (!ProgramParticleRain.Attach(VertexShader)) return false;
	if (!ProgramParticleRain.Attach(FragmentShader)) return false;
	if (!ProgramParticleRain.Link()) return false;
	if (!ProgramParticleRain.Use(true)) return false;

	// Initialise Shader - Water
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramWater.Create()) return false;
	if (!ProgramWater.Attach(VertexShader)) return false;
	if (!ProgramWater.Attach(FragmentShader)) return false;
	if (!ProgramWater.Link()) return false;
	if (!ProgramWater.Use(true)) return false;

	// Initialise Shader - Terrain
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/terrain.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/terrain.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramTerrain.Create()) return false;
	if (!ProgramTerrain.Attach(VertexShader)) return false;
	if (!ProgramTerrain.Attach(FragmentShader)) return false;
	if (!ProgramTerrain.Link()) return false;
	if (!ProgramTerrain.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(ProgramBasic.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(ProgramBasic.GetAttribLocation("aNormal"));

	// --------------------------- MODELS LOAD ---------------------- //

	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\heightmap5.png", 10)) return false;
	if (!water.loadHeightmap("models\\watermap.png", 10)) return false;
	if (!cabin.load("models\\WoodenCabinObj\\WoodenCabinObj.obj")) return false;
	if (!firepit.load("models\\firepit.obj")) return false;
	if (!palmbush.load("models\\palm_bush.obj")) return false;
	if (!pier.load("models\\WoodenBridge_01.obj")) return false;
	if (!mirror.load("models\\mirror.obj")) return false;
	if (!lamp.load("models\\SA_LD_Medieval_Horn_Lantern.obj")) return false;
	mirror.loadMaterials("models\\");

	// --------------------------- SKYBOX LOAD ---------------------- //

	//load skybox
	//if (!skybox.load("models\\Islands\\fd.png",
	//	"models\\Islands\\rt.png",
	//	"models\\Islands\\bk.png",
	//	"models\\Islands\\lt.png",
	//	"models\\Islands\\up.png",
	//	"models\\Islands\\dn.png")) return false;

	//if (!skybox.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
	//	"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
	//	"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
	//	"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
	//	"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
	//	"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;


	if (!skybox.load("models\\FullMoon\\FullMoonFront2048.png",
		"models\\FullMoon\\FullMoonLeft2048.png",
		"models\\FullMoon\\FullMoonBack2048.png",
		"models\\FullMoon\\FullMoonRight2048.png",
		"models\\FullMoon\\FullMoonUp2048.png",
		"models\\FullMoon\\FullMoonDown2048.png")) return false;

	// --------------------------- TEXTURE LOAD ---------------------- //

	// create & load textures
	C3dglBitmap bm;
    glActiveTexture(GL_TEXTURE0);
	
	// Grass texture
	bm.Load("models/grass.png", GL_RGBA);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Sand texture
	bm.Load("models/sand.png", GL_RGBA);
	glGenTextures(1, &idTexSand);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Water texture
	bm.Load("models/water.png", GL_RGBA);
	glGenTextures(1, &idTexWater);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/water_particle.png", GL_RGBA);
	glGenTextures(1, &idTexParticleRain);
	glBindTexture(GL_TEXTURE_2D, idTexParticleRain);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/fire.bmp", GL_RGBA);
	glGenTextures(1, &idTexParticle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexParticle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models\\WoodenCabinObj\\WoodCabinDif.jpg", GL_RGBA);
	glGenTextures(1, &idTexCabin);
	glBindTexture(GL_TEXTURE_2D, idTexCabin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models\\firepit_texture.png", GL_RGBA);
	glGenTextures(1, &idTexFirepit);
	glBindTexture(GL_TEXTURE_2D, idTexFirepit);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models\\palm_bush.png", GL_RGBA);
	glGenTextures(1, &idTexPalmBush);
	glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models\\Wood_01.jpg", GL_RGBA);
	glGenTextures(1, &idTexPier);
	glBindTexture(GL_TEXTURE_2D, idTexPier);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models\\HRN_LMP_CM.bmp", GL_RGBA);
	glGenTextures(1, &idTexLamp);
	glBindTexture(GL_TEXTURE_2D, idTexLamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// cloth texture
	bm.Load("models/cloth.png", GL_RGBA);
	if (!bm.GetBits()) return false;
	glGenTextures(1, &idTexCloth);
	glBindTexture(GL_TEXTURE_2D, idTexCloth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// wood texture
	bm.Load("models/oak.png", GL_RGBA);
	if (!bm.GetBits()) return false;
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Setup the textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	ProgramTerrain.SendUniform("textureBed", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	ProgramTerrain.SendUniform("textureShore", 2);

	// Send the texture info to the shaders
	ProgramBasic.SendUniform("texture0", 0);
	ProgramTerrain.SendUniform("texture0", 0);
	ProgramWater.SendUniform("texture0", 0);
	ProgramParticle.SendUniform("texture0", 0);
	ProgramParticleRain.SendUniform("texture0", 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// --------------------------- LIGHTS SETUP ---------------------- //

	//// Basic Light Setup

	// Ambient
	ProgramBasic.SendUniform("lightAmbient.on", 1);
	ProgramBasic.SendUniform("lightAmbient.color", 0.0, 0.0, 0.0);
	
	// Directional
	ProgramBasic.SendUniform("lightDir.on", 1);
	ProgramBasic.SendUniform("lightDir.direction", -1.0f, 0.2f, 0.0f);
	ProgramBasic.SendUniform("lightDir.diffuse", 0.2, 0.2, 0.2);

	// Fog
	ProgramBasic.SendUniform("fogColour", 0.1, 0.1, 0.1);
	ProgramBasic.SendUniform("fogDensity", 0.1);

	//// Terrain Light Setup

	// Ambient
	ProgramTerrain.SendUniform("lightAmbient.on", 1);
	ProgramTerrain.SendUniform("lightAmbient.color", 0.0, 0.0, 0.0);

	// Directional
	ProgramTerrain.SendUniform("lightDir.on", 1);
	ProgramTerrain.SendUniform("lightDir.direction", -1.0f, 0.2f, 0.0f);
	ProgramTerrain.SendUniform("lightDir.diffuse", 0.2, 0.2, 0.2);

	// Fog
	ProgramTerrain.SendUniform("fogColour", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("fogDensity", 0.1);

	////Per-fragment point light

	ProgramBasic.SendUniform("lightPoint1.on", 1);
	ProgramBasic.SendUniform("lightPoint1.position", 12.0, 8.1, -5.0);
	ProgramBasic.SendUniform("lightPoint1.diffuse", 0.5, 0.5, 0.5);
	ProgramBasic.SendUniform("lightPoint1.specular", 0.0, 0.0, 0.0);

	ProgramBasic.SendUniform("lightPoint2.on", 1);
	ProgramBasic.SendUniform("lightPoint2.position", 11.0, 6.1, -2.0);
	ProgramBasic.SendUniform("lightPoint2.diffuse", 0.1, 0.1, 0.1);
	ProgramBasic.SendUniform("lightPoint2.specular", 0.0, 0.0, 0.0);

	//ProgramBasic.SendUniform("lightPoint3.on", 1);
	//ProgramBasic.SendUniform("lightPoint3.position", 10.0, 5.5, 3.0);
	//ProgramBasic.SendUniform("lightPoint3.diffuse", 0.5, 0.5, 0.5);
	//ProgramBasic.SendUniform("lightPoint3.specular", 0.0, 0.0, 0.0);

	ProgramBasic.SendUniform("lightPoint4.on", 1);
	ProgramBasic.SendUniform("lightPoint4.position", 0.8, 6.3, 4.0);
	ProgramBasic.SendUniform("lightPoint4.diffuse", 0.3, 0.3, 0.3);
	ProgramBasic.SendUniform("lightPoint4.specular", 0.0, 0.0, 0.0);

	ProgramTerrain.SendUniform("lightPoint1.on", 1);
	ProgramTerrain.SendUniform("lightPoint1.position", 12.0, 8.1, -4.0);
	ProgramTerrain.SendUniform("lightPoint1.diffuse", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightPoint1.specular", 0.0, 0.0, 0.0);

	ProgramTerrain.SendUniform("lightPoint2.on", 1);
	ProgramTerrain.SendUniform("lightPoint2.position", 11.0, 6.1, -2.0);
	ProgramTerrain.SendUniform("lightPoint2.diffuse", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightPoint2.specular", 0.0, 0.0, 0.0);

	ProgramTerrain.SendUniform("lightPoint3.on", 1);
	ProgramTerrain.SendUniform("lightPoint3.position", 10.0, 5.5, 3.0);
	ProgramTerrain.SendUniform("lightPoint3.diffuse", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightPoint3.specular", 0.0, 0.0, 0.0);


	// setup materials (for basic and terrain programs only, water does not use these materials):
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	ProgramTerrain.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)
	ProgramTerrain.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// --------------------------- WATER SETUP ---------------------- //

	// setup the water colours and level
	ProgramWater.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramWater.SendUniform("skyColor", 0.2f, 0.6f, 1.f);
	ProgramTerrain.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramTerrain.SendUniform("waterLevel", waterLevel);

	// --------------------------- PARTICLES FIRE SETUP ---------------------- //

	// Setup the particle system

	//ProgramParticle.SendUniform("initialPos", -18.0, 7.6, 3.0);
	ProgramParticle.SendUniform("initialPos", 9.8, 5.3, 3.0);
	ProgramParticle.SendUniform("gravity", 0.0, 0.1, 0.0);
	ProgramParticle.SendUniform("particleLifetime", LIFETIME);

	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;

	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float theta = (float)M_PI / 30.f * (float)rand() / (float)RAND_MAX;
		float phi = (float)M_PI * 1.f * (float)rand() / (float)RAND_MAX;
		float x = sin(theta) * cos(phi);
		float y = 0.00001; 
		float z = sin(theta) * sin(phi);
		float v = 2 + 0.5f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(x * v);
		bufferVelocity.push_back(y * v);
		bufferVelocity.push_back(z * v);

		bufferStartTime.push_back(time);
		time += PERIOD;
	}

	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);
	glGenBuffers(1, &idBufferStartTime); glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// switch on: transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// setup the point size
	glEnable(GL_POINT_SPRITE);
	glPointSize(50);


	// --------------------------- PARTICLES RAIN SETUP ---------------------- //

	// Prepare the particle buffers
	std::vector<float> bufferVelocityRain;
	std::vector<float> bufferStartTimeRain;
	std::vector<float> bufferInitialPosRain;
	float timeRain = 0;
	for (int i = 0; i < NPARTICLES_RAIN; i++)
	{
		float v = 2.5f + 0.2f * (float)rand() / (float)RAND_MAX;

		bufferVelocityRain.push_back(-0.5f * v);
		bufferVelocityRain.push_back(-v);
		bufferVelocityRain.push_back(0.25f * v);
		bufferStartTimeRain.push_back(timeRain);
		timeRain += PERIOD_RAIN;

		bufferInitialPosRain.push_back(80.f * (float)rand() / (float)RAND_MAX - 40);
		bufferInitialPosRain.push_back(12);
		bufferInitialPosRain.push_back(80.f * (float)rand() / (float)RAND_MAX - 40);
	}

	glGenBuffers(1, &idBufferInitialPosRain);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferInitialPosRain);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferInitialPosRain.size(), &bufferInitialPosRain[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferVelocityRain);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocityRain);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocityRain.size(), &bufferVelocityRain[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferStartTimeRain);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTimeRain);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTimeRain.size(), &bufferStartTimeRain[0], GL_STATIC_DRAW);

	// Setup the particle system
	ProgramParticleRain.SendUniform("gravity", 0.0, 0.0, 0.0);
	ProgramParticleRain.SendUniform("particleLifetime", LIFETIME_RAIN);

	glPointSize(50);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// --------------------------- VIEW MATRIX SETUP ---------------------- //

	// Initialise the View Matrix (initial position for the first-person camera)
	glMatrixMode(GL_MODELVIEW);
	angleTilt = 0;
	glLoadIdentity();
	glRotatef(angleTilt, 0, 0, 0);
	//gluLookAt(-22.0, 2.0, 10.0,
	//		  -20.0, 2.0, 0.0,
	//		   0.0, 1.0, 0.0);
	gluLookAt(0.0, 2.0, 10.0,
			  7.0, 2.0, 0.0,
			   0.0, 1.0, 0.0);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void renderObjects() {

	// Basic Shader is not currently in use...
	ProgramBasic.Use();

	//PalmBush 1
	glPushMatrix();
	glTranslatef(6.0, 4.9, -6.0);
	glRotatef(20, 0.0, 1.0, 0.0);
	glScalef(0.015, 0.015, 0.015);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	palmbush.render();
	glPopMatrix();

	//PalmBush 2
	glPushMatrix();
	glTranslatef(-14.0, 4.9, 4.0);
	glRotatef(20, 0.0, 1.0, 0.0);
	glScalef(0.015, 0.015, 0.015);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	palmbush.render();
	glPopMatrix();

	//PalmBush 3
	glPushMatrix();
	glTranslatef(-12.0, 4.9, -14.0);
	glRotatef(20, 0.0, 1.0, 0.0);
	glScalef(0.015, 0.015, 0.015);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	palmbush.render();
	glPopMatrix();

	//PalmBush 4
	glPushMatrix();
	glTranslatef(-4.0, 4.9, -8.0);
	glRotatef(20, 0.0, 1.0, 0.0);
	glScalef(0.015, 0.015, 0.015);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	palmbush.render();
	glPopMatrix();

	////PalmBush 5
	//glPushMatrix();
	//glTranslatef(-20.0, 7.0, 20.0);
	//glRotatef(20, 0.0, 1.0, 0.0);
	//glScalef(0.015, 0.015, 0.015);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	//palmbush.render();
	//glPopMatrix();

	////PalmBush 6
	//glPushMatrix();
	//glTranslatef(0.0, 5.8, 16.0);
	//glRotatef(20, 0.0, 1.0, 0.0);
	//glScalef(0.015, 0.015, 0.015);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	//palmbush.render();
	//glPopMatrix();

	////PalmBush 7
	//glPushMatrix();
	//glTranslatef(0.0, 7.5, -20.0);
	//glRotatef(20, 0.0, 1.0, 0.0);
	//glScalef(0.015, 0.015, 0.015);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	//palmbush.render();
	//glPopMatrix();

	////PalmBush 7
	//glPushMatrix();
	//glTranslatef(-24.0, 7.1, 0.0);
	//glRotatef(20, 0.0, 1.0, 0.0);
	//glScalef(0.015, 0.015, 0.015);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, idTexPalmBush);
	//palmbush.render();
	//glPopMatrix();

	//lamp 1
	glPushMatrix();
	glTranslatef(12.2, 6.2, -0.7);
	glRotatef(-30, 0.0, 1.0, 0.0);
	glScalef(0.90, 0.90, 0.90);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexLamp);
	lamp.render();
	glPopMatrix();

	//lamp 2
	glPushMatrix();
	glTranslatef(2.0, 6.3, 5.0);
	glRotatef(-30, 0.0, 1.0, 0.0);
	glScalef(0.90, 0.90, 0.90);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexLamp);
	lamp.render();
	glPopMatrix();

	//pier
	glPushMatrix();
	glTranslatef(4.0, 4.0, 5.0);
	glRotatef(-30, 0.0, 1.0, 0.0);
	glScalef(0.015, 0.02, 0.02);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexPier);
	pier.render();
	glPopMatrix();

	//cabin
	glPushMatrix();
	glTranslatef(12.0, 5.1, -6.0);
	glRotatef(-30, 0.0, 1.0, 0.0);
	glScalef(0.10, 0.10, 0.10);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexCabin);
	cabin.render();
	glPopMatrix();

	//firepit
	glPushMatrix();
	glTranslatef(10.0, 5.3, 3.0);
	glRotatef(20, 0.0, 1.0, 0.0);
	glScalef(0.6, 0.6, 0.6);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexFirepit);
	firepit.render();
	glPopMatrix();

}

void renderWater() {

	// setup the water texture
	glBindTexture(GL_TEXTURE_2D, idTexWater);

	float matrix[16];
	ProgramWater.Use();
	glPushMatrix();
	glTranslatef(0, waterLevel, 0);
	glScalef(0.5f, 1.0f, 0.5f);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramWater.SendUniform("matrixModelView", matrix);
	water.render();
	glPopMatrix();

}

void renderSkyBox() {

	float matrix[16];

	// render the skybox
	ProgramBasic.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
	skybox.render();
	ProgramBasic.SendUniform("lightAmbient.color", 0.15, 0.15, 0.15);


}

void renderTerrain() {
	float matrix[16];

	// render the terrain
	ProgramTerrain.Use();
	glPushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramTerrain.SendUniform("matrixModelView", matrix);
	terrain.render();
	glPopMatrix();

}

void render()
{
	float matrix[16];
	// send the animation time to shaders
	ProgramWater.SendUniform("t", glutGet(GLUT_ELAPSED_TIME) / 1000.f);
	ProgramParticle.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	// clear screen and buffers
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky colour
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);					// switch tilt off
	glTranslatef(deltaX, deltaY, deltaZ);			// animate camera motion (controlled by WASD keys)
	glRotatef(-angleTilt, 1, 0, 0);					// switch tilt on
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	// set the camera height above the ground
	gluInvertMatrix(matrixView, matrix);
	glTranslatef(0, -max(terrain.getInterpolatedHeight(matrix[12], matrix[14]), waterLevel),  0);

	// setup View Matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramBasic.SendUniform("matrixView", matrix);
	ProgramTerrain.SendUniform("matrixView", matrix);
	ProgramWater.SendUniform("matrixView", matrix);

	//render the skybox
	ProgramBasic.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
	skybox.render();
	ProgramBasic.SendUniform("lightAmbient.color", 0.3, 0.3, 0.3);
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);	
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// render the terrain
	ProgramTerrain.Use();
	glPushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramTerrain.SendUniform("matrixModelView", matrix);
	terrain.render();
	glPopMatrix();

	// setup the water texture
	glBindTexture(GL_TEXTURE_2D, idTexWater);

	// render the water
	ProgramWater.Use();
	glPushMatrix();
	glTranslatef(0, waterLevel, 0);
	glScalef(0.5f, 1.0f, 0.5f);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramWater.SendUniform("matrixModelView", matrix);
	water.render();
	glPopMatrix();

	//renderSkyBox();
	//renderTerrain();
	//renderWater();
	renderObjects();

	ProgramParticle.Use();
	glPointSize(50);


	glDepthMask(GL_FALSE);								// disable depth buffer updates
	glActiveTexture(GL_TEXTURE0);						// choose the active texture
	glBindTexture(GL_TEXTURE_2D, idTexParticle);		// bind the texture

	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramParticle.SendUniform("matrixModelView", matrix);

	// render the buffer
	glEnableVertexAttribArray(0);	// velocity
	glEnableVertexAttribArray(1);	// start time
									//glEnableVertexAttribArray(2);	//iniPos
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, idBufferIniPos);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	//glEnableVertexAttribArray(2);	//iniPos

	glDepthMask(GL_TRUE);

	// setup the rain drop texture
	glBindTexture(GL_TEXTURE_2D, idTexParticleRain);

	///////////////////////////////////
	// RENDER THE PARTICLE SYSTEM

	ProgramParticleRain.Use();
	glPointSize(5);
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexParticleRain);

	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramParticleRain.SendUniform("matrixModelView", matrix);

	ProgramParticleRain.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	// render the buffer
	glEnableVertexAttribArray(0);	// initial position
	glEnableVertexAttribArray(1);	// velocity
	glEnableVertexAttribArray(2);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferInitialPosRain);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocityRain);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTimeRain);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES_RAIN);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// revert to normal
	glDepthMask(GL_TRUE);
	// END OF PARTICLE SYSTEM RENDERING
	///////////////////////////////////

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	// find screen aspect ratio
	float ratio =  w * 1.0f / h;      // we hope that h is not zero

	// setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(60.0, ratio, 0.02, 1000.0);

	float matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, matrix);
	ProgramBasic.SendUniform("matrixProjection", matrix);
	ProgramTerrain.SendUniform("matrixProjection", matrix);
	ProgramWater.SendUniform("matrixProjection", matrix);
	ProgramParticle.SendUniform("matrixProjection", matrix);
	ProgramParticleRain.SendUniform("matrixProjection", matrix);

	ProgramParticle.SendUniform("scaleFactor", (float)h / 720.f);
	ProgramParticleRain.SendUniform("scaleFactor", (float)h / 720.f);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': deltaZ = max(deltaZ * 1.05f, 0.01f); break;
	case 's': deltaZ = min(deltaZ * 1.05f, -0.01f); break;
	case 'a': deltaX = max(deltaX * 1.05f, 0.01f); break;
	case 'd': deltaX = min(deltaX * 1.05f, -0.01f); break;
	case 'e': deltaY = max(deltaY * 1.05f, 0.01f); break;
	case 'q': deltaY = min(deltaY * 1.05f, -0.01f); break;
	}
	// speed limit
	deltaX = max(-0.15f, min(0.15f, deltaX));
	deltaY = max(-0.15f, min(0.15f, deltaY));
	deltaZ = max(-0.15f, min(0.15f, deltaZ));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': deltaZ = 0; break;
	case 'a':
	case 'd': deltaX = 0; break;
	case 'q':
	case 'e': deltaY = 0; break;
	case ' ': deltaY = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	if (state == GLUT_DOWN)
	{
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		glutWarpPointer(cx, cy);
	}
	else
		glutSetCursor(GLUT_CURSOR_INHERIT);
}

// handle mouse move
void onMotion(int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	if (x == cx && y == cy) 
		return;	// caused by glutWarpPointer

	float amp = 0.25;
	float deltaTilt = amp * (y - cy);
	float deltaPan  = amp * (x - cx);

	glutWarpPointer(cx, cy);

	// handle camera tilt (mouse move up & down)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(deltaTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	angleTilt += deltaTilt;

	// handle camera pan (mouse move left & right)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);
	glRotatef(deltaPan, 0, 1, 0);
	glRotatef(-angleTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

