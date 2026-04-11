#pragma once

#include "Common.h"
#include "GameWindow.h"
#include "Texture.h"

// Classes used in game.  For a new class, declare it here and provide a pointer to an object of this class below.  Then, in Game.cpp, 
// include the header.  In the Game constructor, set the pointer to NULL and in Game::Initialise, create a new object.  Don't forget to 
// delete the object in the destructor.   
class CCamera;
class CSkybox;
class CShader;
class CShaderProgram;
class CPlane;
class CFreeTypeFont;
class CHighResolutionTimer;
class CSphere;
class COpenAssetImportMesh;
class CAudio;
class CCatmullRom;
class CCube;
class CCylinder;
class CTexture;

class Game {
private:
	// Three main methods used in the game.  Initialise runs once, while Update and Render run repeatedly in the game loop.
	void Initialise();
	void Update();
	void Render();

	// Pointers to game objects.  They will get allocated in Game::Initialise()
	CSkybox *m_pSkybox;
	CCamera *m_pCamera;
	vector <CShaderProgram *> *m_pShaderPrograms;
	CPlane *m_pPlanarTerrain;
	CFreeTypeFont *m_pFtFont;
	COpenAssetImportMesh *m_pBarrelMesh;
	COpenAssetImportMesh *m_pHorseMesh;
	COpenAssetImportMesh *m_pFenceMesh;
	COpenAssetImportMesh *m_pRedArrowMesh;
	COpenAssetImportMesh *m_pGreenArrowMesh;
	CSphere *m_pSphere;
	CHighResolutionTimer *m_pHighResolutionTimer;
	CAudio *m_pAudio;
	CCatmullRom *m_pCatmullRom;
	CCube *m_pCube;
	CCylinder *m_pCylinder;
	CTexture m_woodTexture;

	// Some other member variables
	double m_dt;
	int m_framesPerSecond;
	bool m_appActive;
	float m_t;
	int m_cameraMode;

	// === Race system ===
	float m_playerSpeed;
	float m_baseSpeed;
	float m_boostSpeed;
	float m_speedDecayRate;

	float m_aiT[4];
	float m_aiSpeeds[4];
	int   m_aiLanes[4];

	int   m_sequence[5];
	int   m_sequenceIndex;
	bool  m_wrongInput;
	float m_flashTimer;

	float m_lapTime;
	int   m_racePosition;
	int   m_playerLane;
	float m_laneWidth;
	float m_lightAngle;
	static const int   BARREL_COUNT = 8;
	float  m_barrelT[BARREL_COUNT];    // t-positions of barrels on track
	float  m_jumpHeight;               // current vertical offset of player
	float  m_jumpVelocity;             // current upward velocity
	bool   m_isJumping;
	int    m_score;                    // increases each barrel successfully passed

public:
	Game();
	~Game();
	static Game& GetInstance();
	LRESULT ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param);
	void SetHinstance(HINSTANCE hinstance);
	WPARAM Execute();

private:
	static const int FPS = 60;
	void DisplayFrameRate();
	void GameLoop();
	void GenerateSequence();
	glm::vec3 GetLanePosition(float t, int lane);
	int ComputeRacePosition();
	GameWindow m_gameWindow;
	HINSTANCE m_hInstance;
	int m_frameCount;
	double m_elapsedTime;

};
