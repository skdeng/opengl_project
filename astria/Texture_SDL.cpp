//==============================================================================
#include "Texture_SDL.h"

//==============================================================================
Texture_SDL::Texture_SDL() {
}

//------------------------------------------------------------------------------
Texture_SDL::~Texture_SDL() {
	if(SDLTexture) {
		SDL_DestroyTexture(SDLTexture);
		SDLTexture = NULL;
	}
}

//==============================================================================
bool Texture_SDL::Load(SDL_Renderer* Renderer, std::string Filename) {
	if(Renderer == NULL) {
		printf("Bad SDL renderer passed");
		return false;
	}

	this->Renderer = Renderer;
	this->Filename = Filename;

	if (fopen(Filename.c_str(), "r") == NULL)
	{
		printf("Unable to load image : %s\nCannot open file", Filename.c_str());
		return false;
	}

	SDL_Surface* TempSurface = IMG_Load(Filename.c_str());
	if(TempSurface == NULL)
	{
		printf("Unable to load image : %s\nError : %s", Filename.c_str(), IMG_GetError());
		return false;
	}

    // Convert SDL surface to a texture
	if((SDLTexture = SDL_CreateTextureFromSurface(Renderer, TempSurface)) == NULL)
	{
		printf("Unable to create SDL Texture : %s\nError : %s", Filename.c_str(), IMG_GetError());
		return false;
	}

    // Grab dimensions
	SDL_QueryTexture(SDLTexture, NULL, NULL, &Width, &Height);

	SDL_FreeSurface(TempSurface);

	return true;
}

bool Texture_SDL::Load(SDL_Renderer* Renderer, std::string Filename, int r, int g, int b)
{
	if (Renderer == NULL) {
		printf("Bad SDL renderer passed");
		return false;
	}

	this->Renderer = Renderer;
	this->Filename = Filename;

	if (fopen(Filename.c_str(), "r") == NULL)
	{
		printf("Unable to load image : %s\nCannot open file", Filename.c_str());
		return false;
	}

	SDL_Surface* TempSurface = IMG_Load(Filename.c_str());
	if (TempSurface == NULL) {
		printf("Unable to load image : %s\nError : %s", Filename.c_str(), IMG_GetError());
		return false;
	}

	SDL_SetColorKey(TempSurface, SDL_TRUE, SDL_MapRGB(TempSurface->format, r, g, b));

	// Convert SDL surface to a texture
	if ((SDLTexture = SDL_CreateTextureFromSurface(Renderer, TempSurface)) == NULL) {
		printf("Unable to create SDL Texture : %s\nError : %s", Filename.c_str(), IMG_GetError());
		return false;
	}

	// Grab dimensions
	SDL_QueryTexture(SDLTexture, NULL, NULL, &Width, &Height);

	SDL_FreeSurface(TempSurface);

	return true;
}

//------------------------------------------------------------------------------
void Texture_SDL::Render(int X, int Y) {
	Render(X, Y, Width, Height);
}

//------------------------------------------------------------------------------
void Texture_SDL::Render(int X, int Y, int Width, int Height) {
	SDL_Rect Destination = {X, Y, Width, Height};

	SDL_RenderCopy(Renderer, SDLTexture, NULL, &Destination);
}

//------------------------------------------------------------------------------
void Texture_SDL::Render(int X, int Y, int Width, int Height, int SX, int SY, int SWidth, int SHeight) {
	SDL_Rect Source = {SX, SY, SWidth, SHeight};
	SDL_Rect Destination = {X, Y, Width, Height};

	SDL_RenderCopy(Renderer, SDLTexture, &Source, &Destination);
}

void Texture_SDL::Render(int X, int Y, double Angle)
{
	SDL_Rect Destination = { X, Y, Width, Height };

	SDL_RenderCopyEx(Renderer, SDLTexture, NULL, &Destination, Angle, NULL, SDL_FLIP_NONE);
}

void Texture_SDL::Render(int X, int Y, int Width, int Height, double Angle)
{
	SDL_Rect Destination = { X, Y, Width, Height };
	SDL_RenderCopyEx(Renderer, SDLTexture, NULL, &Destination, Angle, NULL, SDL_FLIP_NONE);
}

void Texture_SDL::Release()
{
	SDL_DestroyTexture(SDLTexture);
	SDLTexture = NULL;
	Renderer = NULL;
}

void Texture_SDL::SetAlpha(Uint8 alpha)
{
	if (SDL_SetTextureAlphaMod(SDLTexture, alpha) < 0)
		printf("cant change alpha");
}

void Texture_SDL::SetColorMod(int r, int g, int b)
{
	SDL_SetTextureColorMod(SDLTexture, r, g, b);
}

void Texture_SDL::SetBlend(SDL_BlendMode blend)
{
	SDL_SetTextureBlendMode(SDLTexture, blend);
}

//------------------------------------------------------------------------------
int Texture_SDL::GetWidth()  { return Width; }
int Texture_SDL::GetHeight() { return Height; }

SDL_Texture* Texture_SDL::GetTexture(){	return SDLTexture; }

//==============================================================================
