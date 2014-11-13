#include "PauseMenuState.h"
#include "AppStateManager.h"

void CPauseStateOptions::OnActivate()
{
	
}

void CPauseStateOptions::OnDeactivate()
{

}

void CPauseStateOptions::OnEvent(SDL_Event* Event)
{
	CEvent::OnEvent(Event);
	SDL_GetMouseState(&mouseX, &mouseY);

	if (Event->type == SDL_MOUSEMOTION)
	{
		
	}
}

void CPauseStateOptions::OnUpdate()
{

}

void CPauseStateOptions::OnRender()
{
	//Render the snapshot in the background
	SDL_Rect dest = { 0, 0, CMain::GetInstance()->GetWindowWidth(), CMain::GetInstance()->GetWindowHeight() };
	SDL_RenderCopyEx(CMain::GetInstance()->GetRenderer(), CAppStateMain::GetInstance()->GetSnapshot(), 0, &dest, 0, 0, SDL_FLIP_VERTICAL);

}