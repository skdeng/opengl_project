﻿#include "AppStateManager.h"
#include "AppStateMain.h"
#include "Main.h"
#include "utils.h"
#include "Stringify.h"

CAppStateMain CAppStateMain::Instance;

CAppStateMain::CAppStateMain()
{
	Loaded = false;
	snapshot = NULL;
}

CAppStateMain* CAppStateMain::GetInstance()
{
	return &Instance;
}

void CAppStateMain::OnActivate()
{
	bool loadingState = false;
	if (!Loaded)
	{
		Speed = 0.5f;
		MouseSpeed = 0.0015f;
		GravityEnabled = false;
		NumScene = 1;
		CLoadingScreen::OnActivate(&Loaded);
		Loaded = OnLoad();
		SDL_WaitThread(CLoadingScreen::GetThreadID(), NULL);
	}

	//Resume all sound effects for when we return from pause
	CSoundEffect::ResumeAll();

	//Hide mouse cursor
	if (SDL_ShowCursor(SDL_DISABLE) < 0)
	{
		Warning("Unable to hide cursor", "Warning: Unable to hide cursor\n" + string(SDL_GetError()));
	}

	//Center mouse cursor
	SDL_WarpMouseInWindow(CMain::GetInstance()->GetWindow(), CMain::GetInstance()->GetWindowWidth() / 2, CMain::GetInstance()->GetWindowHeight() / 2);

}

void CAppStateMain::OnDeactivate()
{
	//Save a snapshot of the current screen to use in pause menu
	int width = CMain::GetInstance()->GetWindowWidth();
	int height = CMain::GetInstance()->GetWindowHeight();
	SDL_Surface* Surf_Tmp = SDL_CreateRGBSurface(0, width, height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, Surf_Tmp->pixels);
	snapshot = SDL_CreateTextureFromSurface(CMain::GetInstance()->GetRenderer(), Surf_Tmp);
	SDL_FreeSurface(Surf_Tmp);

	//Pause all sound effects
	CSoundEffect::PauseAll();

	SDL_ShowCursor(SDL_ENABLE);
}

void CAppStateMain::OnExit()
{
	if (Loaded)
	{
		std::cout << "Releasing CAppStateMain\n";
		CModel::ReleaseAll();
		CInstancedModel::ReleaseAllInstanced();
		ParticleEruption.Release();	TextureParticleEruption.Release();
		ParticleSmoke.Release();	TextureParticleSmoke.Release();
		ParticleFire.Release();		TextureParticleFire.Release();
		ShaderVertex.Release();
		ShaderFragment.Release();
		ShaderInstancing.Release();
		ProgramMain.Release();
		ProgramInstancing.Release();
		Skybox.Release();
		CHeightMap::ReleaseShaderProgram();
		for (int i = 0; i < 6; i++) TextureTerrain[i].Release();
		Map.Release();
		SoundFire.Release();
		MusicMain.Release();
	}
}

void CAppStateMain::OnEvent(SDL_Event* Event)
{
	if (Event->type == SDL_QUIT)
	{
		CMain::GetInstance()->Running = false;
	}

	CEvent::OnEvent(Event);

	if (Event->type == SDL_MOUSEWHEEL)
	{
		FoV -= 5 * Event->wheel.y;
		Clamp(FoV, 20, 80);
	}

	//Get mouse position
	SDL_GetMouseState(&Mouse_X, &Mouse_Y);

	//Recenter mouse
	SDL_WarpMouseInWindow(CMain::GetInstance()->GetWindow(), CMain::GetInstance()->GetWindowWidth() / 2, CMain::GetInstance()->GetWindowHeight() / 2);

	//Compute new orientation
	HorizontalAngle += MouseSpeed * float(CMain::GetInstance()->GetWindowWidth() / 2 - Mouse_X);
	VerticalAngle += MouseSpeed * float(CMain::GetInstance()->GetWindowHeight() / 2 - Mouse_Y);

}

void CAppStateMain::OnUpdate()
{
	if (MoveUp)		Position += Direction * Speed * CFPS::FPSControl.GetSpeedFactor();
	if (MoveDown)	Position -= Direction * Speed *CFPS::FPSControl.GetSpeedFactor();
	if (MoveLeft)	Position -= Right * Speed *CFPS::FPSControl.GetSpeedFactor();
	if (MoveRight)	Position += Right * Speed *CFPS::FPSControl.GetSpeedFactor();

	if (GravityEnabled)
	{
		Position.y += CParams::Gravity * CFPS::FPSControl.GetSpeedFactor();
		Position.y = max(Position.y, Map.GetHeight(Position) + 5);
	}

	//Direction : Spherical coordinates to cartesian coordinates conversion
	Direction = glm::vec3(
		cos(VerticalAngle) * sin(HorizontalAngle),
		sin(VerticalAngle),
		cos(VerticalAngle) * cos(HorizontalAngle)
		);

	//Right vector
	Right = glm::vec3(
		sin(HorizontalAngle - 3.14f / 2.0f),
		0,
		cos(HorizontalAngle - 3.14f / 2.0f)
		);

	//Up vector
	Up = glm::cross(Right, Direction);

	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 5000.0f);

	//Get the view matrix using lookAt
	ViewMatrix = glm::lookAt(
		Position,
		Position + Direction,
		Up
		);

	//Set matrices for particle program
	ParticleEruption.SetMatrices(&ProjectionMatrix, &ViewMatrix, Direction);
	ParticleSmoke.SetMatrices(&ProjectionMatrix, &ViewMatrix, Direction);
	ParticleFire.SetMatrices(&ProjectionMatrix, &ViewMatrix, Direction);

	float dist = Distance(FirePosition, Position);
	int vol = 1/(dist - 50) * MIX_MAX_VOLUME;
	SoundFire.SetVolume(vol);
}

void CAppStateMain::OnRender()
{
	//Clear the screen for each frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Use shaders
	ProgramMain.Use();

	ProgramMain.SetUniform("matrices.mProjection", &ProjectionMatrix);
	ProgramMain.SetUniform("matrices.mView", &ViewMatrix);
	ProgramMain.SetUniform("mat.diffuse", 0);
	ProgramMain.SetUniform("mat.specular", 2);

	ProgramMain.SetUniform("matrices.mModel", glm::mat4(1.0));
	ProgramMain.SetUniform("matrices.mNormal", glm::mat4(1.0));

	//Set light uniform
	Sun.SetUniform(&ProgramMain, "sunLight");

	//Render Skybox
	ProgramMain.SetUniform("matrices.mModel", glm::translate(glm::mat4(1.0), Position));
	ProgramMain.SetUniform("bSkybox", 1);
	Skybox.Render();
	ProgramMain.SetUniform("bSkybox", 0);

	//Reset model matrix
	ProgramMain.SetUniform("matrices.mModel", glm::mat4(1.0));

	//Render models
	CModel::BindVAO();
	ProgramMain.SetUniform("vEyePosition", Position);

	glm::vec3 newPos(83, 0, 180);
	newPos.y = Map.GetHeight(newPos);
	ModelMatrix = glm::translate(glm::mat4(1.0), newPos);
	ProgramMain.SetModelAndNormalMatrix("matrices.mModel", "matrices.mNormal", ModelMatrix);
	models[0].Render();

	newPos = glm::vec3(64, 0, 193);
	newPos.y = Map.GetHeight(newPos);
	ModelMatrix = glm::translate(glm::mat4(1.0), newPos);
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f, glm::vec3(1, 0, 0));
	ProgramMain.SetModelAndNormalMatrix("matrices.mModel", "matrices.mNormal", ModelMatrix);
	models[1].Render();

	//Render instanced models
	ProgramInstancing.Use();
	ProgramInstancing.SetUniform("matrices.mProjection", ProjectionMatrix);
	ProgramInstancing.SetUniform("matrices.mView", ViewMatrix);
	ProgramInstancing.SetUniform("mat.diffuse", 0);
	ProgramInstancing.SetUniform("mat.specular", 2);
	Sun.SetUniform(&ProgramInstancing, "sunLight");
	SmallTree.RenderInstanced();

	//Render ground
	CShaderProgram* Program_Terrain = CHeightMap::GetShaderProgram();
	Program_Terrain->Use();
	Program_Terrain->SetUniform("matrices.mProjection", ProjectionMatrix);
	Program_Terrain->SetUniform("matrices.mView", ViewMatrix);
	Program_Terrain->SetUniform("vEyePosition", Position);
	Program_Terrain->SetUniform("LowAlt.diffuse", 0);	Program_Terrain->SetUniform("LowAlt.specular", 1);
	Program_Terrain->SetUniform("MidAlt.diffuse", 2);	Program_Terrain->SetUniform("MidAlt.specular", 3);
	Program_Terrain->SetUniform("HighAlt.diffuse", 4);	Program_Terrain->SetUniform("HighAlt.specular", 5);
	Program_Terrain->SetModelAndNormalMatrix("matrices.mModel", "matrices.mNormal", glm::mat4(1.0));
	Program_Terrain->SetUniform("vColor", glm::vec4(1, 1, 1, 1));
	Sun.SetUniform(Program_Terrain, "sunLight");
	//Diffuse textures			//Specular textures
	TextureTerrain[0].Bind(0);	TextureTerrain[1].Bind(1);
	TextureTerrain[2].Bind(2);	TextureTerrain[3].Bind(3);
	TextureTerrain[4].Bind(4);	TextureTerrain[5].Bind(5);
	Map.Render();

	//Render particles
	float fps = CFPS::FPSControl.GetFPS();
	float FrameInterval = 1 / fps;
	TextureParticleEruption.Bind();
	ParticleEruption.Update(FrameInterval);
	ParticleEruption.Render();

	TextureParticleSmoke.Bind();
	ParticleSmoke.Update(FrameInterval);
	ParticleSmoke.Render();

	TextureParticleFire.Bind();
	ParticleFire.Update(FrameInterval);
	ParticleFire.Render();

	SDL_GL_SwapWindow(CMain::GetInstance()->GetWindow());
}

bool CAppStateMain::OnLoad()
{
	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewInitStatus = glewInit();
	if (glewInitStatus != GLEW_OK)
	{
		Error("Loading error", (char*)(glewGetErrorString(glewInitStatus)));
		return false;
	}

	//Disable Vsync to prevent framerate capping at refresh rate
	if (SDL_GL_SetSwapInterval(0) < 0)
	{
		Warning("Loading error", "Warning: Unable to set VSync\n" + string(SDL_GetError()));
	}

	//Clear the background as dark blue
	glClearColor(0.1f, 0.1f, 0.4f, 0.0f);

	//Enable MSAA (4xMSAA as define in the main OnInit() function in Main.cpp)
	//glEnable(GL_MULTISAMPLE);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	//glEnable(GL_LINE_SMOOTH);

	//Enable depth test
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);

	//Remove triangles which normal is not towards the camera (do not render the inside of the model)
	//glEnable(GL_CULL_FACE);

	//Load shaders and programs
	if (!ShaderVertex.Load("shaders/main_shader.vert", GL_VERTEX_SHADER))
		return false;
	if (!ShaderFragment.Load("shaders/main_shader.frag", GL_FRAGMENT_SHADER))
		return false;
	if (!ProgramMain.Initiate(&ShaderVertex, &ShaderFragment))
		return false;
	if (!ShaderInstancing.Load("shaders/instancing.vert", GL_VERTEX_SHADER))
		return false;
	if (!ProgramInstancing.Initiate(&ShaderInstancing, &ShaderFragment))
		return false;

	//Load terrain
	string TextureNames[] = { "sand.jpg", "sand_specular.jpg", "grass.jpg", "grass_specular.jpg", "snow.jpg", "snow_specular.png" };
	for (int i = 0; i < 6; i++)
	{
		TextureTerrain[i].Load_2D("gfx/" + TextureNames[i], true);
		TextureTerrain[i].SetFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	}
	Map.Load(CParams::Heightmap);
	CHeightMap::LoadShaderProgram("shaders/terrain.vert", "shaders/terrain.frag");
	Map.SetSize(CParams::WorldX, CParams::WorldY, CParams::WorldZ);

	//Load models
	models[0].Load("gfx/nanosuit/nanosuit.obj");
	models[1].Load("gfx/barrel/barrel.3ds");

	SmallTree.Load("gfx/coffee_tree/TR08y.obj");

	//Generate some random positions for environment objects
	int SceneObjCount = 75;
	SceneObjPosition.reserve(SceneObjCount);
	for (int i = 0; i < SceneObjCount; i++) SceneObjPosition.push_back(glm::vec3(RandNormal()*CParams::WorldX / 2, 0, RandNormal()*CParams::WorldZ / 2));
	for (int i = 0; i < SceneObjPosition.size(); i++)	SceneObjPosition[i].y = Map.GetHeight(SceneObjPosition[i]);

	ModelMatrices.reserve(SceneObjCount);
	for (int i = 0; i < SceneObjCount; i++)
	{
		ModelMatrix = glm::translate(glm::mat4(1.0), SceneObjPosition[i]);
		ModelMatrix = glm::rotate(ModelMatrix, -90.0f, glm::vec3(1, 0, 0));
		ModelMatrices.push_back(ModelMatrix);
	}
	SmallTree.UploadMatrices(SceneObjCount, ModelMatrices.data());

	CModel::UploadVBO();

	ModelMatrices.clear(); ModelMatrices.shrink_to_fit();

	//Load particles
	TextureParticleEruption.Load_2D("gfx/particle.bmp", true);
	ParticleEruption.Init();
	FirePosition = glm::vec3(64, 0, 193);
	FirePosition.y = Map.GetHeight(FirePosition);
	ParticleEruption.Set(
		FirePosition,				//Position
		glm::vec3(-8, 15, -8),		//Minimum velocity
		glm::vec3(8, 20, 8),		//Maximum velocity
		glm::vec3(0, -10, 0),		//Acceleration
		glm::vec3(0.8, 0.3, 0.1),	//Color
		1.5f,	//Minimum lifespan in second
		2.0f,	//Maximum lifespan in second
		0.25f, 	//Size
		0.02,	//Spawn interval
		1000);	//Count i.e. number generated per frame

	TextureParticleFire.Load_2D("gfx/img/FireParticle.jpg", true);
	ParticleFire.Init();
	ParticleFire.Set(
		FirePosition,
		glm::vec3(-7, 8, -7),
		glm::vec3(7, 20, 7),
		glm::vec3(0, -8, 0),
		glm::vec3(0.3, 0.3, 0.3),
		0.5f,
		1.5f,
		2.0f,
		0.2,
		50);

	FirePosition.y += 3;
	TextureParticleSmoke.Load_2D("gfx/img/SmokeParticle.jpg", true);
	ParticleSmoke.Init();
	ParticleSmoke.Set(
		FirePosition,
		glm::vec3(-5, 8, -5),
		glm::vec3(5, 20, 5),
		glm::vec3(0, -8, 0),
		glm::vec3(0.3, 0.3, 0.5),
		1.5f,
		2.0f,
		2.0f,
		0.2,
		10);

	//Used for wire frame
	PolyMode = GL_FILL;

	//Model matrix //Identity matrix
	ModelMatrix = glm::mat4(1.0f);

	//Load the skybox
	Skybox.Load(CParams::SkyboxFolder);

	//Load sun light
	Sun = CDirectLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-0.1, -1.2, 1), 0.2f, 0.8f);

	//Load camera properties
	Position = glm::vec3(-78, 135, 76);
	Position.y = 5 + Map.GetHeight(Position);
	FoV = 45.0f;
	HorizontalAngle = 0.5f;
	VerticalAngle = -0.1f;

	MusicMain.Load("sound/PortalSelfEsteemFund.mp3");
	MusicMain.Play();

	SoundFire.Load("sound/fireplace.wav", 1);
	SoundFire.Play(-1);

	return true;
}

SDL_Texture* CAppStateMain::GetSnapshot()
{
	return snapshot;
}

void CAppStateMain::OnKeyDown(SDL_Keycode sym, Uint16 mod, SDL_Scancode scancode)
{
	string pos;
	switch (sym)
	{
		//Terminate the program if the user press Esc or LeftAlt+F4
	case SDLK_ESCAPE:
		CAppStateManager::SetActiveAppState(APPSTATE_PAUSE);
		break;
	case SDLK_F4:
		if (scancode == SDL_SCANCODE_LALT)
		{
			CMain::GetInstance()->Running = false;
		}
		break;
	case SDLK_F2:
		//Faster movement speed for quicker debugging xD
		Speed = (Speed == 0.5) ? 5 : 0.5;
		break;
	case SDLK_F3:
		//Disable gravity
		GravityEnabled = !GravityEnabled;
		break;
	case SDLK_F12:
		//Prompt current position
		pos = Stringify::Float(Position.x) + " " + Stringify::Float(Position.y) + " " + Stringify::Float(Position.z) + "\n";
		Warning("Position", pos);
		break;
	case SDLK_PAGEUP:
		NumScene = min(NumScene + 1, 3);
		break;
	case SDLK_PAGEDOWN:
		NumScene = max(NumScene - 1, 0);
		break;

		//Movement Keys
	case SDLK_w:
		MoveUp = true;	//Movement bool variable is needed to counteract the inherent key repeat delay
		break;
	case SDLK_s:
		MoveDown = true;
		break;
	case SDLK_d:
		MoveRight = true;
		break;
	case SDLK_a:
		MoveLeft = true;
		break;

		//Switch between normal model and wireframe
	case SDLK_q:
		if (PolyMode == GL_FILL)
		{
			PolyMode = GL_LINE;
			glPolygonMode(GL_FRONT_AND_BACK, PolyMode);
		}
		else if (PolyMode == GL_LINE)	//Useless but pretty cool
		{
			PolyMode = GL_FILL;
			glPolygonMode(GL_FRONT_AND_BACK, PolyMode);
		}
		break;
	}
}

void CAppStateMain::OnKeyUp(SDL_Keycode sym, Uint16 mod, SDL_Scancode scancode)
{
	switch (sym)
	{
	case SDLK_w:
		MoveUp = false;
		break;
	case SDLK_s:
		MoveDown = false;
		break;
	case SDLK_d:
		MoveRight = false;
		break;
	case SDLK_a:
		MoveLeft = false;
		break;
	}
}

void CAppStateMain::OnLoseFocus()
{
	CAppStateManager::SetActiveAppState(APPSTATE_PAUSE);
}