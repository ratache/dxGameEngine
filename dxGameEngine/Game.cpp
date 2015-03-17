#include "Game.h"

/*	ScrollingBackground setup
********************************/
int ScrollX, ScrollY; //current scroll position
int SpeedX, SpeedY; //scroll speed
LPDIRECT3DSURFACE9 gameworld; //scroll buffer
long start; //timing variable


bool Game_Init(HWND hwnd)
{
	Init_DirectInput(hwnd);
	Init_Keyboard(hwnd);
	Init_Mouse(hwnd);

	start = GetTickCount();
	
	return 1;
}

/* <-GAME RUN->
There are two parts to the Game_Run function. The first part should move and
animate the sprite(s) in the game. The second part should draw the sprite(s) to
the screen. The reason that a screen update is divided into two parts (one for
logic, the other for screen refresh) is because you don’t want to take too much
processing time in between the BeginScene and EndScene calls, so keep the code
there to the minimum required to update the graphics and leave other processing
tasks for either before or after the screen update.
*/
void Game_Run(HWND hWnd)
{
	//////////////////////////////////////////////////////////////////////////////
	// REACT TO USER INPUT
	/////////////////////////////////////////////////////////////////////////////
	//poll DirectInput devices
	Poll_Keyboard();
	Poll_Mouse();
	//check for escape key (to exit program)
	if (Key_Down(DIK_ESCAPE))
		PostMessage(hWnd, WM_DESTROY, 0, 0);
	//scroll based on mouse input
	if (Mouse_X() != 0) ScrollX += Mouse_X();
	if (Mouse_Y() != 0) ScrollY += Mouse_Y();
	
	//////////////////////////////////////////////////////////////////////////////
	// UPDATE AND RENDER GAME
	/////////////////////////////////////////////////////////////////////////////
	
	//after short delay, ready for next frame?
	//this keeps the game running at a steady frame rate
	if (GetTickCount() - start >= 30)
	{
		//reset timing
		start = GetTickCount();
	}
		//start rendering
	//	if (d3ddev->BeginScene())
	//	{
	//		//update the scrolling view
	//		//stop rendering
	//		d3ddev->EndScene();
	//	}
	//}
	////display the back buffer on the screen
	//d3ddev->Present(NULL, NULL, NULL, NULL);
}

void Game_End(HWND hWnd)
{
	if (gameworld != NULL){
		gameworld->Release();
	}
	Kill_Keyboard();
	Kill_Mouse();
	dinput->Release(); 
}