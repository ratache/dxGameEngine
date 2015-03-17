#include "dxGraphics.h"

//variables
RECT screenresolution;
HWND hWnd;
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
D3DPRESENT_PARAMETERS d3dpp;
LPDIRECT3DSURFACE9 backBuffer = NULL;//Backbuffer is where we paint before displaying.
LPDIRECT3DSURFACE9 frontBuffer = NULL;//Frontbuffer is the surface displayed
LPD3DXSPRITE sprite_handler;
D3DXVECTOR3 cameraSource;
D3DXVECTOR3 cameraTarget;

bool Init_Graphics(HWND hwnd, bool fs)//Constructor
{
	GetClientRect(hwnd, &screenresolution);
	hWnd = hwnd;

	//initialize Direct3D
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
	{
		MessageBox(hWnd, "Error initializing Direct3D", "Constructor error", MB_OK);
		return 0;
	}
	SetMode(screenresolution.right, screenresolution.bottom, fs);
	return 1;
}

bool SetMode(int x, int y, bool fs){
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = !fs;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = x;
	d3dpp.BackBufferHeight = y;

	//create Direct3D device
	d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);

	if (d3ddev == NULL)
	{
		MessageBox(hWnd, "Error creating Direct3D device", "Error", MB_OK);
		return 0;
	}
	//clear the backbuffer to black
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	//create pointer to the back buffer
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);
	//return okay
	return 1;
}

RECT GetMode(){
	return screenresolution;
}

void Clear(DWORD color)
{
	int a, r, g, b;
	r = color >> 16 & 0xff;
	g = color >> 8 & 0xff;
	b = color & 0xff;

	//make sure the Direct3D device is valid
	if (d3ddev == NULL)
		return;
	//set backbuffer to specified color
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(r, g, b), 1.0f, 0);
	//paint
	if (d3ddev->BeginScene())
	{
		d3ddev->EndScene();
	}

	d3ddev->Present(NULL, NULL, NULL, NULL);
}

void DrawImage(char const* pathInParam, int x, int y, float scale){
	RECT pos;//Position rect
	LPDIRECT3DSURFACE9 img;
	D3DXIMAGE_INFO img_info;
	D3DXGetImageInfoFromFile(pathInParam, &img_info);
	int width = img_info.Width * scale;
	int height = img_info.Height * scale;//desired scaled size

	img = LoadSurface(pathInParam);

	//make sure the Direct3D device is valid
	if (d3ddev == NULL)
	{
		MessageBox(hWnd, "Error creating Direct3D device","DrawImage Error", MB_OK);
		return;
	}

	pos.left = x;
	pos.top = y;
	pos.right = width; //* drawThis.getScale();//Width and height is scaled according to image obj properties
	pos.bottom = height; //* drawThis.getScale();

	//start rendering
	if (d3ddev->BeginScene())//<Tells Direct3D we are about to paint, locks up VRAM
	{//Now we can use VRAM data>
		d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);//create pointer to the back buffer
		d3ddev->StretchRect(img, NULL, backBuffer, &pos, D3DTEXF_NONE);	//draw surface to the backbuffer
		d3ddev->EndScene();//stop rendering, unlock VRAM
	}
	d3ddev->Present(NULL, NULL, NULL, NULL);//display the back buffer on the screen (nullparameters are bout swapchains and window handling)
}

void DrawText(char const* text, int textSize, int x, int y, DWORD color){
	HRESULT result;
	HDC hdc = GetDC(0); // hämta skärmens DC
	int logicalHeight = -MulDiv(textSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	LPD3DXFONT font;

	int a, r, g, b;
	a = color >> 24 & 0xff;
	r = color >> 16 & 0xff;
	g = color >> 8 & 0xff;
	b = color & 0xff;

	result = D3DXCreateFont(
		d3ddev,
		logicalHeight,
		0, // bredden för fonten, eller 0 för att låta Windows bestämma
		0, // ”vikt”, till exempel fet stil
		1, // mip-nivåer
		0, // 1 = kursiv stil
		ANSI_CHARSET,
		OUT_TT_ONLY_PRECIS, // använd endast true-type fonter
		0, // ”kvalité”, används ej för true-type
		0, // ?
		"Times New Roman", // fontnamnet
		&font);

	RECT rect = { x, y, 0, 0 };//<-startpunkt

	//start rendering
	if (d3ddev->BeginScene())//<Tells Direct3D we are about to paint, locks up VRAM
	{//Now we can use VRAM data>
		font->DrawText(
			0,
			text,
			-1,
			&rect,
			DT_NOCLIP,
			D3DCOLOR_ARGB(a, r, g, b));
		d3ddev->EndScene();//stop rendering, unlock VRAM
	}
	d3ddev->Present(NULL, NULL, NULL, NULL);//display the back buffer on the screen (nullparameters are bout swapchains and window handling)

}

/*
This function draws a sprite.
Sprite creation works this way:
*create a sprite and load a bitmap into the sprite surface.
*control the animation of the sprite.
*move the sprite on the screen.
These should be supported by the class SpriteState.
121*/
void DrawSprite(LPDIRECT3DTEXTURE9 sprite[], int x, int y, int currentframe, LPDIRECT3DSURFACE9 back){
	//start rendering
	if (d3ddev->BeginScene())
	{
		d3ddev->StretchRect(back, NULL, backBuffer, NULL, D3DTEXF_NONE);
		//start sprite handler
		sprite_handler->Begin(D3DXSPRITE_ALPHABLEND);

		//create vector to update sprite position
		D3DXVECTOR3 position(x, y, 0);

		//draw the sprite
		sprite_handler->Draw(
			sprite[currentframe],
			NULL,
			NULL,
			&position,
			D3DCOLOR_XRGB(255, 255, 255));

		//stop drawing
		sprite_handler->End();

		//stop rendering
		d3ddev->EndScene();
	}
	//display the back buffer on the screen
	d3ddev->Present(NULL, NULL, NULL, NULL);
}

void DrawSprites(LPDIRECT3DTEXTURE9 sprite_array[], Sprite sprite_info[], int sprites, int currentframe, LPDIRECT3DSURFACE9 back){
	//start rendering
	if (d3ddev->BeginScene())
	{
		d3ddev->StretchRect(back, NULL, backBuffer, NULL, D3DTEXF_NONE);
		//start sprite handler
		sprite_handler->Begin(D3DXSPRITE_ALPHABLEND);
		int n;
		D3DXVECTOR3 position(0, 0, 0); //ball position vector
		//draw the balls
		for (n = 0; n<sprites; n++)
		{
			position.x = (float)sprite_info[n].getPosX();
			position.y = (float)sprite_info[n].getPosY();
			sprite_handler->Draw(
				sprite_array[currentframe],
				NULL,
				NULL,
				&position,
				D3DCOLOR_XRGB(255, 255, 255));
		}

		//stop drawing
		sprite_handler->End();

		//stop rendering
		d3ddev->EndScene();
	}
	//display the back buffer on the screen
	d3ddev->Present(NULL, NULL, NULL, NULL);
}

/*
Load surface
Requires parameters:
int Width, Height
char const* Path
*/
LPDIRECT3DSURFACE9 LoadSurface(char const* pathInParam, D3DCOLOR transcolor)
{
	LPDIRECT3DSURFACE9 image = NULL;
	D3DXIMAGE_INFO info;
	HRESULT result;
	result = D3DXGetImageInfoFromFile(pathInParam, &info);
	if (result != D3D_OK)
		return NULL;

	//prep surface for delivery
	result = d3ddev->CreateOffscreenPlainSurface(
		info.Width,
		info.Height,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&image,
		NULL);

	if (result != D3D_OK)
		return NULL;

	//load picture into surface
	result = D3DXLoadSurfaceFromFile(
		image,
		NULL,
		NULL,
		pathInParam,//loads the picture from path
		NULL,
		D3DX_DEFAULT,
		transcolor,
		NULL);

	//make sure file was loaded okay
	if (result != D3D_OK)
		return NULL;

	return image;
}

LPDIRECT3DTEXTURE9 LoadTexture(char *filename, D3DCOLOR transcolor){
	LPDIRECT3DTEXTURE9 texture = NULL;//Texture for foreground objects

	//the struct for reading bitmap file info
	D3DXIMAGE_INFO info;

	//standard Windows return value
	HRESULT result;

	//get width and height from bitmap file
	result = D3DXGetImageInfoFromFile(filename, &info);
	if (result != D3D_OK)
		return NULL;

	//create the new texture by loading a bitmap image file
	D3DXCreateTextureFromFileEx(
		d3ddev,              //Direct3D device object
		filename,            //bitmap filename
		info.Width,          //bitmap image width
		info.Height,         //bitmap image height
		1,                   //mip-map levels (1 for no chain)
		D3DPOOL_DEFAULT,     //the type of surface (standard)
		D3DFMT_UNKNOWN,      //surface format (default)
		D3DPOOL_DEFAULT,     //memory class for the texture
		D3DX_DEFAULT,        //image filter
		D3DX_DEFAULT,        //mip filter
		transcolor,          //color key for transparency
		&info,               //bitmap file info (from loaded file)
		NULL,                //color palette
		&texture);          //destination texture

	//make sure the bitmap textre was loaded correctly
	if (result != D3D_OK)
		return NULL;

	return texture;
}

/*
Sprite setup
Transparency color is pink.
*/
bool Sprite_setup(LPDIRECT3DTEXTURE9 sprite_array[], char const *filename, int arraySize){
	char s[20];
	int n;

	//create sprite handler object
	HRESULT result = D3DXCreateSprite(d3ddev, &sprite_handler);
	if (result != D3D_OK)
		return 0;

	//load the sprite animation
	for (n = 0; n<arraySize; n++)
	{
		//printf("%d", sizeof(sprite_array + 1));
		//set up the filename
		//sprintf(s, "cat%d.bmp", n + 1);
		sprintf_s(s, "%s%d.bmp", filename, n + 1);
		//load texture with "pink" as the transparent color
		sprite_array[n] = LoadTexture(s, D3DCOLOR_XRGB(255, 0, 255));
		if (sprite_array[n] == NULL){
			MessageBox(hWnd, "Sprite setup failed.", "Array sprite error", MB_OK);
			return 0;
		}
	}
	return 1;
}

void SetPosition(QUAD *quad, int ivert, float x, float y, float z)
{
	quad->vertices[ivert].x = x;
	quad->vertices[ivert].y = y;
	quad->vertices[ivert].z = z;
}

void SetVertex(QUAD *quad, int ivert, float x, float y, float z, float tu, float tv)
{
	SetPosition(quad, ivert, x, y, z);
	quad->vertices[ivert].tu = tu;
	quad->vertices[ivert].tv = tv;
}

VERTEX CreateVertex(float x, float y, float z, float tu, float tv)
{
	VERTEX vertex;
	vertex.x = x;
	vertex.y = y;
	vertex.z = z;
	vertex.tu = tu;
	vertex.tv = tv;

	return vertex;
}

QUAD *CreateQuad(char *textureFilename)
{
	QUAD *quad = (QUAD*)malloc(sizeof(QUAD));
	//load the texture
	D3DXCreateTextureFromFile(d3ddev, textureFilename, &quad->texture);
	//create the vertex buffer for this quad
	d3ddev->CreateVertexBuffer(
		4 * sizeof(VERTEX),
		0,
		D3DFVF_MYVERTEX, D3DPOOL_DEFAULT,
		&quad->buffer,
		NULL);
	//create the four corners of this dual triangle strip
	//each vertex is X,Y,Z and the texture coordinates U,V
	quad->vertices[0] = CreateVertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	quad->vertices[1] = CreateVertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	quad->vertices[2] = CreateVertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	quad->vertices[3] = CreateVertex(1.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	return quad;
}

void DeleteQuad(QUAD *quad)
{
	if (quad == NULL)
		return;
	//free the vertex buffer
	if (quad->buffer != NULL)
		quad->buffer->Release();
	//free the texture
	if (quad->texture != NULL)
		quad->texture->Release();
	//free the quad
	free(quad);
}

void DrawQuad(QUAD *quad)
{
	//fill vertex buffer with this quad’s vertices
	void *temp = NULL;
	quad->buffer->Lock(0, sizeof(quad->vertices), (void**)&temp, 0);
	memcpy(temp, quad->vertices, sizeof(quad->vertices));
	quad->buffer->Unlock();
	//draw the textured dual triangle strip
	d3ddev->SetTexture(0, quad->texture);
	d3ddev->SetStreamSource(0, quad->buffer, 0, sizeof(VERTEX));
	d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

void SetIdentity()
{
	//set default position, scale, and rotation
	D3DXMATRIX matWorld;
	D3DXMatrixTranslation(&matWorld, 0.0f, 0.0f, 0.0f);
	d3ddev->SetTransform(D3DTS_WORLD, &matWorld);
}

void ClearScene(D3DXCOLOR color)
{
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
}

void SetCamera(float x, float y, float z, float lookx, float looky, float lookz)
{
	D3DXMATRIX matView;
	D3DXVECTOR3 updir(0.0f, 1.0f, 0.0f);
	//move the camera
	cameraSource.x = x;
	cameraSource.y = y;
	cameraSource.z = z;
	//point the camera
	cameraTarget.x = lookx;
	cameraTarget.y = looky;
	cameraTarget.z = lookz;
	//set up the camera view matrix
	D3DXMatrixLookAtLH(&matView, &cameraSource, &cameraTarget, &updir);
	d3ddev->SetTransform(D3DTS_VIEW, &matView);
}

void SetPerspective(float fieldOfView, float aspectRatio, float nearRange,
	float farRange)
{
	//set the perspective so things in the distance will look smaller
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, fieldOfView, aspectRatio, nearRange,
		farRange);
	d3ddev->SetTransform(D3DTS_PROJECTION, &matProj);
}

void Release_All_Graphics(){
	d3d->Release();
	d3ddev->Release();
	if (sprite_handler != NULL){
		sprite_handler->Release();
	}
	if (backBuffer != NULL){
		backBuffer->Release();
	}
}
