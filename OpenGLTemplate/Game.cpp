/* 
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting, 
 different camera controls, different shaders, etc.
 
 Template version 5.0a 29/01/2017
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk) 

 version 6.0a 29/01/2019
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

 version 6.1a 13/02/2022 - Sorted out Release mode and a few small compiler warnings
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

*/


#include "game.h"

// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "Sphere.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Audio.h"
#include "Texture.h"

#include "CatmullRom.h"
#include "Cube.h"
#include "Cylinder.h"

// Constructor
Game::Game()
{
	m_pSkybox = NULL;
	m_pCamera = NULL;
	m_pShaderPrograms = NULL;
	m_pPlanarTerrain = NULL;
	m_pFtFont = NULL;
	m_pBarrelMesh = NULL;
	m_pHorseMesh = NULL;
	m_pFenceMesh = NULL;
	m_pRedArrowMesh  = NULL;
	m_pGreenArrowMesh = NULL;
	m_pSphere = NULL;
	m_pHighResolutionTimer = NULL;
	m_pAudio = NULL;
	m_pCatmullRom = NULL;
	m_pCube = NULL;
	m_pCylinder = NULL;

	m_dt = 0.0;
	m_framesPerSecond = 0;
	m_frameCount = 0;
	m_elapsedTime = 0.0f;
	m_t = 0.0f;
	m_cameraMode = 0;
	m_lightAngle = 0.0f;
	m_jumpHeight   = 0.0f;
	m_jumpVelocity = 0.0f;
	m_isJumping    = false;
	m_score        = 0;
}

// Destructor
Game::~Game() 
{ 
	//game objects
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
	delete m_pBarrelMesh;
	delete m_pHorseMesh;
	delete m_pFenceMesh;
	delete m_pRedArrowMesh;
	delete m_pGreenArrowMesh;
	delete m_pSphere;
	delete m_pAudio;
	delete m_pCatmullRom;
	delete m_pCube;
	delete m_pCylinder;

	if (m_pShaderPrograms != NULL) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pHighResolutionTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise() 
{
	// Set the clear colour and depth
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	/// Create objects
	m_pCamera = new CCamera;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector <CShaderProgram *>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pBarrelMesh = new COpenAssetImportMesh;
	m_pHorseMesh = new COpenAssetImportMesh;
	m_pSphere = new CSphere;
	m_pAudio = new CAudio;
	m_pCatmullRom = new CCatmullRom;
	m_pCube = new CCube;
	m_pCylinder = new CCylinder;

	// Catmull-Rom spline path
	glm::vec3 p0 = glm::vec3(     0.0f, 20.0f,  12000.0f);
	glm::vec3 p1 = glm::vec3( 12000.0f, 20.0f,      0.0f);
	glm::vec3 p2 = glm::vec3(     0.0f, 20.0f, -12000.0f);
	glm::vec3 p3 = glm::vec3(-12000.0f, 20.0f,      0.0f);
	m_pCatmullRom->CreatePath(p0, p1, p2, p3);

	// === Race setup ===
	m_baseSpeed      = 0.0000140f;
	m_boostSpeed     = 0.0000230f;
	m_playerSpeed    = m_baseSpeed;
	m_speedDecayRate = 0.00000000325f;
	m_playerLane     = 0;
	m_laneWidth      = 400.0f;

	m_aiT[0] = 0.005f;  m_aiSpeeds[0] = 0.0000180f;  m_aiLanes[0] = -2;
	m_aiT[1] = 0.010f;  m_aiSpeeds[1] = 0.0000160f;  m_aiLanes[1] = -1;
	m_aiT[2] = 0.003f;  m_aiSpeeds[2] = 0.0000175f;  m_aiLanes[2] =  1;
	m_aiT[3] = 0.007f;  m_aiSpeeds[3] = 0.0000165f;  m_aiLanes[3] =  2;

	m_sequenceIndex = 0;
	m_wrongInput    = false;
	m_flashTimer    = 0.0f;
	m_lapTime       = 0.0f;
	m_racePosition  = 1;

	srand((unsigned int)time(NULL));
	GenerateSequence();
	// Spread 8 barrels evenly around the track in the player's lane
	for (int i = 0; i < BARREL_COUNT; i++)
		m_barrelT[i] = (float)(i + 1) / (float)(BARREL_COUNT + 1);

	RECT dimensions = m_gameWindow.GetDimensions();

	int width = dimensions.right - dimensions.left;
	int height = dimensions.bottom - dimensions.top;

	// Set the orthographic and perspective projection matrices based on the image size
	m_pCamera->SetOrthographicProjectionMatrix(width, height); 
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float)width / (float)height, 0.5f, 100000.0f);

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");

	sShaderFileNames.push_back("cubeShader.vert");
	sShaderFileNames.push_back("cubeShader.frag");
	sShaderFileNames.push_back("toonShader.vert");
	sShaderFileNames.push_back("toonShader.frag");

	for (int i = 0; i < (int) sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int) sShaderFileNames[i].size()-4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
		string sShaderPath = (sShaderFileNames[i].find("toonShader") == 0)
			? "Shaders\\" + sShaderFileNames[i]
			: "resources\\shaders\\" + sShaderFileNames[i];
		shader.LoadShader(sShaderPath, iShaderType);
		shShaders.push_back(shader);
	}

	// Create the main shader program
	CShaderProgram *pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	// Create a shader program for fonts
	CShaderProgram *pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	// Create a shader program for the cube
	CShaderProgram* pCubeProgram = new CShaderProgram;
	pCubeProgram->CreateProgram();
	pCubeProgram->AddShaderToProgram(&shShaders[4]);
	pCubeProgram->AddShaderToProgram(&shShaders[5]);
	pCubeProgram->LinkProgram();
	m_pShaderPrograms->push_back(pCubeProgram);

	// Create toon shader program
	CShaderProgram *pToonProgram = new CShaderProgram;
	pToonProgram->CreateProgram();
	pToonProgram->AddShaderToProgram(&shShaders[6]);
	pToonProgram->AddShaderToProgram(&shShaders[7]);
	pToonProgram->LinkProgram();
	m_pShaderPrograms->push_back(pToonProgram);

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
	m_pSkybox->Create(50000.0f);
	
	// Create the planar terrain
	m_pPlanarTerrain->Create("resources\\textures\\", "grassfloor01.jpg", 40000.0f, 40000.0f, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

	m_pFtFont->LoadSystemFont("arial.ttf", 32);
	m_pFtFont->SetShaderProgram(pFontProgram);

	// Load some meshes in OBJ format
	m_pBarrelMesh->Load("resources\\models\\Barrel\\Barrel02.obj");  // Downloaded from http://www.psionicgames.com/?page_id=24 on 24 Jan 2013
	m_pHorseMesh->Load("resources\\models\\Horse\\Horse2.obj");  // Downloaded from http://opengameart.org/content/horse-lowpoly on 24 Jan 2013
	m_pFenceMesh = new COpenAssetImportMesh;
	m_pFenceMesh->Load("resources\\models\\Fence\\fence.obj"); // Downloaded from https://sketchfab.com/3d-models/wood-fence-aac4aefc4cf848178ca2bb3a03cacb35 on 30 March 2026
	m_pRedArrowMesh = new COpenAssetImportMesh;
	m_pRedArrowMesh->Load("resources\\models\\Arrows\\red arrow.obj");
	m_pGreenArrowMesh = new COpenAssetImportMesh;
	m_pGreenArrowMesh->Load("resources\\models\\Arrows\\green arrow.obj");
	m_woodTexture.Load("resources\\textures\\wood.jpg");

	// Create a sphere
	m_pSphere->Create("resources\\textures\\", "dirtpile01.jpg", 25, 25);  // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013
	glEnable(GL_CULL_FACE);

	// Initialise audio and play background music
	m_pAudio->Initialise();
	m_pAudio->LoadEventSound("resources\\Audio\\Boing.wav");					// Royalty free sound from freesound.org
	m_pAudio->LoadMusicStream("resources\\Audio\\DST-Garote.mp3");	// Royalty free music from http://www.nosoapradio.us/
	m_pAudio->PlayMusicStream();
	m_pAudio->SetMusicVolume(0.3f);

	// Create a cube
	m_pCube->Create("resources\\textures\\", "pog.jpg");
	m_pCylinder->Create("resources\\textures\\", "wood.jpg", 1.0f, 1.0f, 16);
}

// Render method runs repeatedly in a loop
void Game::Render() 
{
	
	// Clear the buffers and enable depth testing (z-buffering)
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Set up a matrix stack
	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	// Use the main shader program 
	CShaderProgram *pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);
	// Note: cubemap and non-cubemap textures should not be mixed in the same texture unit.  Setting unit 10 to be a cubemap texture.
	int cubeMapTextureUnit = 10; 
	pMainProgram->SetUniform("CubeMapTex", cubeMapTextureUnit);
	

	// Set the projection matrix
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	// Call LookAt to create the view matrix and put this on the modelViewMatrix stack. 
	// Store the view matrix and the normal matrix associated with the view matrix for later (they're useful for lighting -- since lighting is done in eye coordinates)
	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 viewNormalMatrix = m_pCamera->ComputeNormalMatrix(viewMatrix);

	
	// Set light and materials in main shader program
	glm::vec4 lightPosition1 = glm::vec4(-100, 100, -100, 1); // Position of light source *in world coordinates*
	pMainProgram->SetUniform("light1.position", viewMatrix*lightPosition1); // Position of light source *in eye coordinates*
	pMainProgram->SetUniform("light1.La", glm::vec3(1.0f));		// Ambient colour of light
	pMainProgram->SetUniform("light1.Ld", glm::vec3(1.0f));		// Diffuse colour of light
	pMainProgram->SetUniform("light1.Ls", glm::vec3(1.0f));		// Specular colour of light
	pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.0f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.0f));	// Specular material reflectance
	pMainProgram->SetUniform("material1.shininess", 15.0f);		// Shininess material property
	pMainProgram->SetUniform("fogColour", glm::vec3(0.55f, 0.6f, 0.65f));
	pMainProgram->SetUniform("fogStart",  1000.0f);
	pMainProgram->SetUniform("fogEnd",    10000.0f);

	// Dynamic orbiting light
	float orbitRadius = 10000.0f;
	float orbitHeight = 4000.0f;
	glm::vec4 light2WorldPos(
		orbitRadius * cos(m_lightAngle),
		orbitHeight,
		orbitRadius * sin(m_lightAngle),
		1.0f
	);
	pMainProgram->SetUniform("light2.position", viewMatrix * light2WorldPos);

	// Blend factor: 0.0 = base speed (natural white), 1.0 = full boost (golden)
	float boostT = glm::clamp(
		(m_playerSpeed - m_baseSpeed) / (m_boostSpeed - m_baseSpeed),
		0.0f, 1.0f
	);
	glm::vec3 goldenAmbient(0.18f, 0.12f, 0.02f);
	glm::vec3 goldenDiffuse(1.0f,  0.72f, 0.08f);
	glm::vec3 goldenSpecular(1.0f, 0.92f, 0.45f);
	glm::vec3 naturalAmbient(0.15f, 0.15f, 0.15f);
	glm::vec3 naturalDiffuse(1.0f,  1.0f,  0.95f);
	glm::vec3 naturalSpecular(1.0f, 1.0f,  1.0f);
	pMainProgram->SetUniform("light2.La", glm::mix(naturalAmbient,  goldenAmbient,  boostT));
	pMainProgram->SetUniform("light2.Ld", glm::mix(naturalDiffuse,  goldenDiffuse,  boostT));
	pMainProgram->SetUniform("light2.Ls", glm::mix(naturalSpecular, goldenSpecular, boostT));


	// Render the skybox and terrain with full ambient reflectance 
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("renderSkybox", true);
		// Translate the modelview matrix to the camera eye point so skybox stays centred around camera
		glm::vec3 vEye = m_pCamera->GetPosition();
		modelViewMatrixStack.Translate(vEye);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pSkybox->Render(cubeMapTextureUnit);
		pMainProgram->SetUniform("renderSkybox", false);
	modelViewMatrixStack.Pop();

	// Render the planar terrain
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pPlanarTerrain->Render();
	modelViewMatrixStack.Pop();


	// Turn on diffuse + specular materials
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance	


	// Switch to toon shader for horses and barrels
	CShaderProgram* pToonProgram = (*m_pShaderPrograms)[3];
	pToonProgram->UseProgram();
	pToonProgram->SetUniform("sampler0", 0);
	pToonProgram->SetUniform("bUseTexture", true);
	pToonProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
	pToonProgram->SetUniform("light1.position", viewMatrix * glm::vec4(-100, 100, -100, 1));
	pToonProgram->SetUniform("light1.La", glm::vec3(1.0f));
	pToonProgram->SetUniform("light1.Ld", glm::vec3(1.0f));
	pToonProgram->SetUniform("light1.Ls", glm::vec3(1.0f));

	// Draw AI horses
	for (int i = 0; i < 4; i++) {
		glm::vec3 aiPos = GetLanePosition(m_aiT[i], m_aiLanes[i]);
		float tA = m_aiT[i] + 0.005f;
		if (tA > 1.0f) tA -= 1.0f;
		glm::vec3 aiCenterNow = GetLanePosition(m_aiT[i], 0);
		glm::vec3 aiCenterAhead = GetLanePosition(tA, 0);
		glm::vec3 aiForward = glm::normalize(aiCenterAhead - aiCenterNow);
		float aiAngle = atan2(aiForward.x, aiForward.z) + 1.570796f;

		modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(aiPos);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), aiAngle);
			modelViewMatrixStack.Scale(62.5f);
			pToonProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pToonProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
			m_pHorseMesh->Render();
		modelViewMatrixStack.Pop();
	}

	for (int i = 0; i < BARREL_COUNT; i++) {
		glm::vec3 barrelPos = GetLanePosition(m_barrelT[i], 0);
		barrelPos.y += 20.0f;  // sit on track surface

		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(barrelPos);
		modelViewMatrixStack.Scale(187.5f);
		pToonProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pToonProgram->SetUniform("matrices.normalMatrix",
			m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pBarrelMesh->Render();
		modelViewMatrixStack.Pop();
	}

	glm::vec3 playerRenderPos = GetLanePosition(m_t, m_playerLane);
	float tAhead = m_t + 0.005f;
	if (tAhead > 1.0f) tAhead -= 1.0f;
	glm::vec3 playerCenterNow = GetLanePosition(m_t, 0);
	glm::vec3 playerCenterAhead = GetLanePosition(tAhead, 0);
	glm::vec3 playerForward = glm::normalize(playerCenterAhead - playerCenterNow);
	float playerAngle = atan2(playerForward.x, playerForward.z) + 1.570796f;

	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(playerRenderPos + glm::vec3(0.0f, m_jumpHeight, 0.0f));
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), playerAngle);
		modelViewMatrixStack.Scale(62.5f);
		pToonProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pToonProgram->SetUniform("matrices.normalMatrix",
			m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pHorseMesh->Render();
	modelViewMatrixStack.Pop();
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("sampler0", 0);
	pMainProgram->SetUniform("bUseTexture", true);

	// Draw 5 sequence arrows floating above the track ahead of player
	glm::vec3 arrowUp(0.0f, 1.0f, 0.0f);
	glm::vec3 arrowCenter = playerRenderPos + playerForward * 1800.0f;
	glm::vec3 arrowRight = glm::normalize(glm::cross(playerForward, arrowUp));
	float arrowSplineAngle = atan2(playerForward.x, playerForward.z);

	for (int i = 0; i < 5; i++) {
		glm::vec3 arrowPos = arrowCenter + arrowRight * ((float)i - 2.0f) * 450.0f;

		// Float above the track
		arrowPos.y += 600.0f;

		// Green if already inputted, red if not yet done
		COpenAssetImportMesh* arrowMesh =
			(i < m_sequenceIndex) ? m_pGreenArrowMesh : m_pRedArrowMesh;

		// Current arrow to press is larger
		float arrowScale = (i == m_sequenceIndex) ? 225.0f : 150.0f;

		// Rotate arrow to show direction: UP=3.15, DOWN=0, LEFT=1.575, RIGHT=4.725
		float dirAngles[] = { 3.15f, 0.0f, 1.575f, 4.725f };
		float dirAngle = dirAngles[m_sequence[i]];

		modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(arrowPos);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), arrowSplineAngle);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 0.0f, 1.0f), dirAngle);
			modelViewMatrixStack.Scale(arrowScale);
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix",
				m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
			arrowMesh->Render();
		modelViewMatrixStack.Pop();
	}


	
	// Render the barrel 
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(  8000.0f, 20.0f,     0.0f));
		modelViewMatrixStack.Scale(15.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pBarrelMesh->Render();
	modelViewMatrixStack.Pop();

	// barrel 2
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(     0.0f, 20.0f,  8000.0f));
		modelViewMatrixStack.Scale(15.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pBarrelMesh->Render();
	modelViewMatrixStack.Pop();

	// barrel 3
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3( -8000.0f, 20.0f,     0.0f));
		modelViewMatrixStack.Scale(15.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pBarrelMesh->Render();
	modelViewMatrixStack.Pop();

	m_woodTexture.Bind(0);
	const int fenceCount = 120;
	for (int i = 0; i < fenceCount; i++) {
		float t = (float)i / (float)fenceCount;
		glm::vec3 outerFencePos = GetLanePosition(t, 5);
		glm::vec3 innerFencePos = GetLanePosition(t, -5);
		glm::vec3 centerPos = GetLanePosition(t, 0);

		glm::vec3 outerToCenter = glm::normalize(centerPos - outerFencePos);
		float outerFaceAngle = atan2(outerToCenter.x, outerToCenter.z) + 1.570796f;

		glm::vec3 innerToCenter = glm::normalize(centerPos - innerFencePos);
		float innerFaceAngle = atan2(innerToCenter.x, innerToCenter.z) + 1.570796f;

		modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(outerFencePos);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), outerFaceAngle);
			modelViewMatrixStack.Scale(187.5f);
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix",
				m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
			m_pFenceMesh->Render();
		modelViewMatrixStack.Pop();

		modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(innerFencePos);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), innerFaceAngle);
			modelViewMatrixStack.Scale(187.5f);
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix",
				m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
			m_pFenceMesh->Render();
		modelViewMatrixStack.Pop();
	}

	const int postCount = 120;
	for (int i = 0; i < postCount; i++) {
		float t = (i + 0.5f) / (float)postCount;

		glm::vec3 outerPostPos = GetLanePosition(t, 5);
		float tAOuter = t + 0.005f; if (tAOuter > 1.0f) tAOuter -= 1.0f;
		glm::vec3 outerPos2 = GetLanePosition(tAOuter, 5);
		glm::vec3 outerFwd = glm::normalize(outerPos2 - outerPostPos);
		float outerYAngle = atan2(outerFwd.x, outerFwd.z);

		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(outerPostPos);
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), outerYAngle);
		modelViewMatrixStack.Scale(glm::vec3(60.0f, 375.0f, 60.0f));
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix",
			m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pCylinder->Render();
		modelViewMatrixStack.Pop();

		glm::vec3 innerPostPos = GetLanePosition(t, -5);
		float tAInner = t + 0.005f; if (tAInner > 1.0f) tAInner -= 1.0f;
		glm::vec3 innerPos2 = GetLanePosition(tAInner, -5);
		glm::vec3 innerFwd = glm::normalize(innerPos2 - innerPostPos);
		float innerYAngle = atan2(innerFwd.x, innerFwd.z);

		modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(innerPostPos);
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), innerYAngle);
		modelViewMatrixStack.Scale(glm::vec3(60.0f, 375.0f, 60.0f));
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix",
			m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pCylinder->Render();
		modelViewMatrixStack.Pop();
	}

	// Render the sphere
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(0.0f, 20.0f, 11000.0f));
		modelViewMatrixStack.Scale(2.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pSphere->Render();
	modelViewMatrixStack.Pop();

	// larger sphere
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3( 11000.0f, 20.0f,     0.0f));
		modelViewMatrixStack.Scale(2.0f);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		// To turn off texture mapping and use the sphere colour only (currently white material), uncomment the next line
		//pMainProgram->SetUniform("bUseTexture", false);
		m_pSphere->Render();
	modelViewMatrixStack.Pop();

	// Render the cube
	CShaderProgram* pCubeProgram = (*m_pShaderPrograms)[2];
	pCubeProgram->UseProgram();
	pCubeProgram->SetUniform("bUseTexture", true);
	pCubeProgram->SetUniform("sampler0", 0);
	pCubeProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
	pCubeProgram->SetUniform("light1.position", viewMatrix * glm::vec4(-100, 100, -100, 1));
	pCubeProgram->SetUniform("light1.La", glm::vec3(0.2f));
	pCubeProgram->SetUniform("light1.Ld", glm::vec3(1.0f));
	pCubeProgram->SetUniform("light1.Ls", glm::vec3(1.0f, 1.0f, 0.0f));
	pCubeProgram->SetUniform("material1.Ma", glm::vec3(0.2f));
	pCubeProgram->SetUniform("material1.Md", glm::vec3(0.8f));
	pCubeProgram->SetUniform("material1.Ms", glm::vec3(1.0f));
	pCubeProgram->SetUniform("material1.shininess", 200.0f);
	modelViewMatrixStack.Push();
		modelViewMatrixStack.Translate(glm::vec3(0.0f, 20.0f,0.0f));
		modelViewMatrixStack.Scale(5.0f);
		pCubeProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pCubeProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pCube->Render();
	modelViewMatrixStack.Pop();

	// Draw the 2D graphics after the 3D graphics
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	SwapBuffers(m_gameWindow.Hdc());		

}

void Game::GenerateSequence()
{
	for (int i = 0; i < 5; i++)
		m_sequence[i] = rand() % 4; // 0=UP 1=DOWN 2=LEFT 3=RIGHT
	m_sequenceIndex = 0;
}

glm::vec3 Game::GetLanePosition(float t, int lane)
{
	glm::vec3 p0(     0.0f, 20.0f,  12000.0f);
	glm::vec3 p1( 12000.0f, 20.0f,      0.0f);
	glm::vec3 p2(     0.0f, 20.0f, -12000.0f);
	glm::vec3 p3(-12000.0f, 20.0f,      0.0f);
	glm::vec3 points[4] = { p0, p1, p2, p3 };

	auto sampleSpline = [&](float sT) -> glm::vec3 {
		float total = sT * 4.0f;
		int seg = (int)total % 4;
		float local = total - (int)total;
		glm::vec3 a = points[(seg + 0) % 4];
		glm::vec3 b = points[(seg + 1) % 4];
		glm::vec3 c = points[(seg + 2) % 4];
		glm::vec3 d = points[(seg + 3) % 4];
		return m_pCatmullRom->Interpolate(a, b, c, d, local);
	};

	glm::vec3 pos = sampleSpline(t);
	float tA = t + 0.005f;
	if (tA > 1.0f) tA -= 1.0f;
	glm::vec3 ahead   = sampleSpline(tA);
	glm::vec3 forward = glm::normalize(ahead - pos);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 right   = glm::normalize(glm::cross(forward, up));
	return pos + right * (float(lane) * m_laneWidth);
}

int Game::ComputeRacePosition()
{
	int position = 1;
	for (int i = 0; i < 4; i++)
		if (m_aiT[i] > m_t) position++;
	return position;
}

void Game::Update() 
{	
	// === Speed decay ===
	m_playerSpeed -= m_speedDecayRate * (float)m_dt;
	if (m_playerSpeed < m_baseSpeed) m_playerSpeed = m_baseSpeed;

	// === Advance player along track ===
	m_t += m_playerSpeed * (float)m_dt;
	m_lightAngle += (float)m_dt * 0.00065f;
	if (m_lightAngle > glm::two_pi<float>()) m_lightAngle -= glm::two_pi<float>();
	if (m_t > 1.0f) m_t -= 1.0f;

	// === Advance AI horses ===
	for (int i = 0; i < 4; i++) {
		m_aiT[i] += m_aiSpeeds[i] * (float)m_dt;
		if (m_aiT[i] > 1.0f) m_aiT[i] -= 1.0f;
	}

	// === Flash timer ===
	if (m_wrongInput) {
		m_flashTimer -= (float)m_dt;
		if (m_flashTimer <= 0.0f) m_wrongInput = false;
	}

	// === Jump physics ===
	const float gravity      = 0.0014f;
	const float jumpStrength = 0.55f;
	if (m_isJumping) {
		m_jumpVelocity -= gravity * (float)m_dt;
		m_jumpHeight   += m_jumpVelocity * (float)m_dt;
		if (m_jumpHeight <= 0.0f) {
			m_jumpHeight   = 0.0f;
			m_jumpVelocity = 0.0f;
			m_isJumping    = false;
		}
	}

	// === Barrel collision detection ===
	glm::vec3 playerPosCol = GetLanePosition(m_t, m_playerLane);
	playerPosCol.y += m_jumpHeight;
	const float barrelCollisionRadius = 300.0f;
	const float safeJumpHeight        = 200.0f;

	for (int i = 0; i < BARREL_COUNT; i++) {
		glm::vec3 barrelPos = GetLanePosition(m_barrelT[i], 0);
		float dist = glm::length(glm::vec3(playerPosCol.x - barrelPos.x,
										   0.0f,
										   playerPosCol.z - barrelPos.z));
		if (dist < barrelCollisionRadius && m_jumpHeight < safeJumpHeight) {
			// Hit a barrel -- slow down and flash
			m_playerSpeed  = m_baseSpeed * 0.75f;
			m_wrongInput   = true;
			m_flashTimer   = 500.0f;
		}
		// Award score when player passes a barrel safely (barrel is behind player)
		float diff = m_t - m_barrelT[i];
		if (diff > 0.005f && diff < 0.025f && m_jumpHeight >= safeJumpHeight) {
			m_score++;
		}
	}

	// === Lap timer ===
	m_lapTime += (float)m_dt / 1000.0f;

	// === Race position ===
	m_racePosition = ComputeRacePosition();

	// === Camera ===
	glm::vec3 playerPos = GetLanePosition(m_t, m_playerLane);
	float tAhead = m_t + 0.005f;
	if (tAhead > 1.0f) tAhead -= 1.0f;
	glm::vec3 ahead   = GetLanePosition(tAhead, m_playerLane);
	glm::vec3 forward = glm::normalize(ahead - playerPos);
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	if (m_cameraMode == 0) {
		glm::vec3 fpPos = playerPos + up * 420.0f;
		m_pCamera->Set(fpPos, fpPos + forward * 4000.0f, up);
	} else if (m_cameraMode == 1) {
		glm::vec3 camPos = playerPos - forward * 1500.0f + up * 600.0f;
		m_pCamera->Set(camPos, playerPos, up);
	} else {
		m_pCamera->Update(m_dt);
	}

	m_pAudio->Update();
}



void Game::DisplayFrameRate()
{
	// Increase the elapsed time and frame counter
	m_elapsedTime += m_dt;
	m_frameCount++;

	if (m_elapsedTime > 1000)
    {
		m_elapsedTime = 0;
		m_framesPerSecond = m_frameCount;

		// Reset the frames per second
		m_frameCount = 0;
    }

	CShaderProgram *pFontProgram = (*m_pShaderPrograms)[1];
	RECT dimensions = m_gameWindow.GetDimensions();
	int width  = dimensions.right  - dimensions.left;
	int height = dimensions.bottom - dimensions.top;
	pFontProgram->UseProgram();
	pFontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
	pFontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
	pFontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	glDisable(GL_DEPTH_TEST);

	m_pFtFont->Render(20, height - 40, 20, "FPS: %d", m_framesPerSecond);

	int lapMins = (int)(m_lapTime / 60.0f);
	float lapSecs = fmod(m_lapTime, 60.0f);
	m_pFtFont->Render(20, height - 70, 20, "Time: %d:%04.1f", lapMins, lapSecs);

	const char* suffixes[] = { "st", "nd", "rd", "th", "th" };
	int posIndex = glm::clamp(m_racePosition - 1, 0, 4);
	m_pFtFont->Render(20, height - 100, 20, "Position: %d%s", m_racePosition, suffixes[posIndex]);

	float boostPct = ((m_playerSpeed - m_baseSpeed) / (m_boostSpeed - m_baseSpeed)) * 100.0f;
	boostPct = glm::clamp(boostPct, 0.0f, 100.0f);
	m_pFtFont->Render(20, height - 130, 20, "Boost: %.0f%%", boostPct);

	m_pFtFont->Render(20, height - 160, 20, "Sequence: %d/5", m_sequenceIndex);
	m_pFtFont->Render(20, height - 190, 20, "Score: %d", m_score);

	if (m_wrongInput) {
		pFontProgram->SetUniform("vColour", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		m_pFtFont->Render(width / 2 - 60, height / 2, 40, "WRONG!");
		pFontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	glEnable(GL_DEPTH_TEST);
}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	/*
	// Fixed timer
	dDt = pHighResolutionTimer->Elapsed();
	if (dDt > 1000.0 / (double) Game::FPS) {
		pHighResolutionTimer->Start();
		Update();
		Render();
	}
	*/
	
	
	// Variable timer
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();
	

}


WPARAM Game::Execute() 
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hInstance);

	if(!m_gameWindow.Hdc()) {
		return 1;
	}

	Initialise();

	m_pHighResolutionTimer->Start();

	
	MSG msg;

	while(1) {											
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			if(msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		} else if (m_appActive) {
			GameLoop();
		} 
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	m_gameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param) 
{
	LRESULT result = 0;

	switch (message) {


	case WM_ACTIVATE:
	{
		switch(LOWORD(w_param))
		{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				m_appActive = true;
				m_pHighResolutionTimer->Start();
				break;
			case WA_INACTIVE:
				m_appActive = false;
				break;
		}
		break;
		}

	case WM_SIZE:
			RECT dimensions;
			GetClientRect(window, &dimensions);
			m_gameWindow.SetDimensions(dimensions);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;

	case WM_KEYDOWN:
		switch(w_param) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case 'C':
			m_cameraMode = (m_cameraMode + 1) % 3;
			break;
		case '1':
			m_pAudio->PlayEventSound();
			break;
		case VK_F1:
			m_pAudio->PlayEventSound();
			break;
		case VK_SPACE:
			if (!m_isJumping) {
				m_isJumping    = true;
				m_jumpVelocity = 1.375f;
			}
			break;
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
		{
			int pressed = (w_param == VK_UP)   ? 0 :
					          (w_param == VK_DOWN)  ? 1 :
					          (w_param == VK_LEFT)  ? 2 : 3;
			if (pressed == m_sequence[m_sequenceIndex]) {
				m_sequenceIndex++;
				if (m_sequenceIndex == 5) {
					m_playerSpeed = m_boostSpeed;
					GenerateSequence();
				}
			} else {
				m_sequenceIndex = 0;
				m_wrongInput    = true;
				m_flashTimer    = 300.0f;
			}
			break;
		}
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(window, message, w_param, l_param);
		break;
	}

	return result;
}

Game& Game::GetInstance() 
{
	static Game instance;

	return instance;
}

void Game::SetHinstance(HINSTANCE hinstance) 
{
	m_hInstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int) 
{
	Game &game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return int(game.Execute());
}