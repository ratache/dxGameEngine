#ifndef quad_h
#define quad_h

#include <d3d9.h>
#include "vertex.h"

struct QUAD
{
	VERTEX vertices[4];
	LPDIRECT3DVERTEXBUFFER9 buffer;
	LPDIRECT3DTEXTURE9 texture;
};

#endif //