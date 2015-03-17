#ifndef _DXGRAPHICS_H
#define _DXGRAPHICS_H
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <time.h>
#include "dxSprite.h"
#include "quad.h"

#define D3DFVF_MYVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

extern D3DXVECTOR3 cameraSource;
extern D3DXVECTOR3 cameraTarget;

bool Init_Graphics(HWND, bool fullscreen = false);
bool SetMode(int, int, bool); // sets resolution, width*height, fullscreen.	
RECT GetMode();
void Flip(bool); // switch sceen / backbuffer. D3DPresent vertical retrace.
void Clear(DWORD color = 0); // Clears backbuffer to spec color (black default).
void DrawImage(char const*, int x = 0, int y = 0, float = 1.0);
void DrawText(char const*, int, int, int, DWORD); //Draw text at x,y, size.
void DrawSprite(LPDIRECT3DTEXTURE9 sprite_array[], int, int, int, LPDIRECT3DSURFACE9);
void DrawSprites(LPDIRECT3DTEXTURE9 sprite_array[], Sprite sprite_info[], int, int, LPDIRECT3DSURFACE9);
LPDIRECT3DSURFACE9 LoadSurface(char const* pathInParam, D3DCOLOR transcolor = (255, 0, 255));
LPDIRECT3DTEXTURE9 LoadTexture(char *filename, D3DCOLOR transcolor = (255, 0, 255));
void Release_All_Graphics();
bool Sprite_setup(LPDIRECT3DTEXTURE9 sprite_array[], char const*, int);
//3D
void SetPosition(QUAD*, int, float, float, float);
void SetVertex(QUAD*, int, float, float, float, float, float);
VERTEX CreateVertex(float, float, float, float, float);
QUAD* CreateQuad(char*);
void DeleteQuad(QUAD*);
void DrawQuad(QUAD*);
void SetIdentity();
void SetCamera(float, float, float, float, float, float);
void SetPerspective(float, float, float, float);
void ClearScene(D3DXCOLOR);
#endif