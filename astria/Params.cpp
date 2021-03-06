#include "Params.h"

char* CParams::File = "params.ini";

//Set everything before loading the values from .ini file
//In case the file loading fails
double CParams::Pi = 3.14159265358979;
double CParams::HalfPi = Pi / 2;
double CParams::TwoPi = Pi * 2;
char CParams::WindowName[64] = "ASTRIA";
char CParams::VersionNumber[16] = "d0.0.0";
int CParams::WindowWidth = 480;
int CParams::WindowHeight = 360;
float CParams::AspectRatio = 4.0f / 3.0f;
char CParams::IconImage[128] = "gfx/img/icon.png";
char CParams::IntroImage[128] = "gfx/img/splash.jpg";
int CParams::IntroDelay = 2000;
char CParams::SkyboxFolder[128] = "/gfx/skybox";
char CParams::Heightmap[128] = "/gfx/2.jpg";
float CParams::WorldX = 300;
float CParams::WorldY =  30;
float CParams::WorldZ = 300;
float CParams::GroundLevel = 0;
float CParams::Gravity = -1;
double CParams::ActivationResponse = 0;
double CParams::Bias = 0;
double CParams::CrossoverRate = 0;
double CParams::MutationRate = 0;
double CParams::MaxPerturbation = 0;

//Load the paramets from the file
//Return false if failed to load file
bool CParams::Load(char* file)
{
	std::ifstream inFile(file);

	//Verify file openings
	if (!inFile)
	{
		return false;
	}

	//Load from the file
	char ParamDescription[40];

	inFile >> ParamDescription; inFile.getline(WindowName, 40);
	inFile >> ParamDescription >> VersionNumber;
	inFile >> ParamDescription >> WindowWidth;
	inFile >> ParamDescription >> WindowHeight;
	inFile >> ParamDescription >> IconImage;
	inFile >> ParamDescription >> IntroImage;
	inFile >> ParamDescription >> IntroDelay;
	inFile >> ParamDescription >> SkyboxFolder;
	inFile >> ParamDescription >> Heightmap;
	inFile >> ParamDescription >> WorldX;
	inFile >> ParamDescription >> WorldY;
	inFile >> ParamDescription >> WorldZ;
	inFile >> ParamDescription >> GroundLevel;
	inFile >> ParamDescription >> Gravity;
	inFile >> ParamDescription >> ActivationResponse;
	inFile >> ParamDescription >> Bias;
	inFile >> ParamDescription >> CrossoverRate;
	inFile >> ParamDescription >> MutationRate;
	inFile >> ParamDescription >> MaxPerturbation;

	AspectRatio = WindowWidth / WindowHeight;

	return true;
}

void CParams::Store(char* file)
{
	std::ofstream outFile(file);

	if (!outFile)	return;

	char ParamDescription[40];

	outFile << ParamDescription << ' ' << WindowName << std::endl;
	outFile << ParamDescription << ' ' << VersionNumber << std::endl;
	outFile << ParamDescription << ' ' << WindowWidth << std::endl;
	outFile << ParamDescription << ' ' << WindowHeight << std::endl;
	outFile << ParamDescription << ' ' << IconImage << std::endl;
	outFile << ParamDescription << ' ' << IntroImage << std::endl;
	outFile << ParamDescription << ' ' << IntroDelay << std::endl;
	outFile << ParamDescription << ' ' << SkyboxFolder << std::endl;
	outFile << ParamDescription << ' ' << Heightmap << std::endl;
	outFile << ParamDescription << ' ' << WorldX << std::endl;
	outFile << ParamDescription << ' ' << WorldY << std::endl;
	outFile << ParamDescription << ' ' << WorldZ << std::endl;
	outFile << ParamDescription << ' ' << GroundLevel << std::endl;
	outFile << ParamDescription << ' ' << Gravity << std::endl;
	outFile << ParamDescription << ' ' << ActivationResponse << std::endl;
	outFile << ParamDescription << ' ' << Bias << std::endl;
	outFile << ParamDescription << ' ' << CrossoverRate << std::endl;
	outFile << ParamDescription << ' ' << MutationRate << std::endl;
	outFile << ParamDescription << ' ' << MaxPerturbation;
}