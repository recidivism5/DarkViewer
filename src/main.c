#include "../DarkEngine/src/DarkEngine.h"
#include "res.h"

WCHAR exePath[MAX_PATH];
WCHAR currentFolder[MAX_PATH];
WCHAR gpath[MAX_PATH+16];
HWND gwnd;
HDC hdc;
int clientWidth = 800, clientHeight = 600, menuHeight = 32, workspaceHeight;
Mat4 ortho;
CachedFont font;
typedef struct {
	GLuint vertexBuffer;
	int vertexCount;
} CachedMesh;
CachedMesh imageQuad;
Texture texture;
HCURSOR cursorArrow, cursorFinger, cursorPan;
ListHeader *images;
int imageIndex;
int scale = 1;
bool interpolation = false;
FVec3 pos;
bool pan = false;
POINT panPoint;
FVec3 originalPos;

ListHeader *GetImagesInFolder(WCHAR *path){
	WIN32_FIND_DATAW fd;
	HANDLE hFind = 0;
	if ((hFind = FindFirstFileW(path,&fd)) == INVALID_HANDLE_VALUE) FatalErrorW(L"GetImagesInFolder: Folder not found: %s",path);
	ListHeader *names;
	ListInit(&names,sizeof(WCHAR *),0);
	do {
		//FindFirstFile will always return "." and ".." as the first two directories.
		if (wcscmp(fd.cFileName,L".") && wcscmp(fd.cFileName,L"..") && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			size_t len = wcslen(fd.cFileName);
			if (len > 4 && (!wcscmp(fd.cFileName+len-4,L".jpg") || !wcscmp(fd.cFileName+len-4,L".png"))){
				char *s = MallocOrDie((len+1)*sizeof(WCHAR));
				wcscpy(s,fd.cFileName);
				ListAppend(&names,&s,1);
			}
		}
	} while (FindNextFileW(hFind,&fd));
	FindClose(hFind);
	return names;
}

void LoadImg(WCHAR *path){
	TextureFromFileW(&texture,path,interpolation);
	_snwprintf(gpath,COUNT(gpath),L"%s - DarkViewer",path);
	SetWindowTextW(gwnd,gpath);
	wcscpy(gpath,path);
	wcscpy(currentFolder,path);
	WCHAR *s = wcsrchr(gpath,L'\\');
	currentFolder[s-gpath+1] = 0;
	WCHAR *name = (s-gpath)+path+1;
	s[1] = L'*';
	s[2] = 0;
	if (images) free(images);
	images = GetImagesInFolder(gpath);
	printf("images count: %d\n",images->used);
	for (int i = 0; i < images->used; i++){
		s = *((WCHAR **)ListGet(images,i));
		if (!wcscmp(s,name)){
			imageIndex = i;
			printf("imageIndex: %d\n",i);
			break;
		}
	}
}

bool PointInButton(int buttonX, int buttonY, int halfWidth, int halfHeight, int x, int y){
	return abs(x-buttonX) < halfWidth && abs(y-buttonY) < halfHeight;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
		case WM_CREATE:{
			DragAcceptFiles(hwnd,1);
			gwnd = hwnd;
			hdc = GetDC(hwnd);
			RECT wr;
			GetWindowRect(hwnd,&wr);
			SetWindowPos(hwnd,0,wr.left,wr.top,wr.right-wr.left,wr.bottom-wr.top,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);//this prevents the single white frame when the window first appears
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:{
			clientWidth = LOWORD(lParam);
			clientHeight = HIWORD(lParam);
			workspaceHeight = clientHeight - menuHeight;
			glViewport(0,0,clientWidth,clientHeight);
			ortho = mat4Ortho(0,clientWidth,0,clientHeight,-10,10);
			return 0;
		}
		case WM_MOUSEMOVE:{
			int x = GET_X_LPARAM(lParam);
			int y = clientHeight - GET_Y_LPARAM(lParam) - 1;
			if (pan){
				pos.x = originalPos.x + (x - panPoint.x);
				pos.y = originalPos.y + (y - panPoint.y);
				InvalidateRect(hwnd,0,0);
			}
			if (PointInButton(20,workspaceHeight/2,10,25,x,y) || PointInButton(clientWidth-20,workspaceHeight/2,10,25,x,y) || PointInButton(4+50,clientHeight-16,50,10,x,y)){
				SetCursor(cursorFinger);
			} else SetCursor(scale == 1 ? cursorArrow : cursorPan);
			return 0;
		}
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:{
			int x = GET_X_LPARAM(lParam);
			int y = clientHeight - GET_Y_LPARAM(lParam) - 1;
			if (PointInButton(20,workspaceHeight/2,10,25,x,y)){
				if (imageIndex > 0){
					imageIndex--;
					_snwprintf(gpath,COUNT(gpath),L"%s%s",currentFolder,*((WCHAR **)ListGet(images,imageIndex)));
					TextureFromFileW(&texture,gpath,interpolation);
					SetWindowTextW(gwnd,gpath);
					InvalidateRect(hwnd,0,0);
				}
			} else if (PointInButton(clientWidth-20,workspaceHeight/2,10,25,x,y)){
				if (imageIndex < images->used-1){
					imageIndex++;
					_snwprintf(gpath,COUNT(gpath),L"%s%s",currentFolder,*((WCHAR **)ListGet(images,imageIndex)));
					TextureFromFileW(&texture,gpath,interpolation);
					SetWindowTextW(gwnd,gpath);
					InvalidateRect(hwnd,0,0);
				}
			} else if (PointInButton(4+50,clientHeight-16,50,10,x,y)){
				interpolation = !interpolation;
				glBindTexture(GL_TEXTURE_2D,texture.id);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,interpolation ? GL_LINEAR : GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,interpolation ? GL_LINEAR : GL_NEAREST);
				InvalidateRect(hwnd,0,0);
			} else if (y < workspaceHeight && scale > 1){
				panPoint.x = x;
				panPoint.y = y;
				originalPos = pos;
				pan = true;
			}
			return 0;
		}
		case WM_DROPFILES:{
			WCHAR p[MAX_PATH];
			if (DragQueryFileW(wParam,0xFFFFFFFF,0,0) > 1) goto EXIT_DROPFILES;
			DragQueryFileW(wParam,0,p,COUNT(p));
			LoadImg(p);
		EXIT_DROPFILES:
			DragFinish(wParam);
			InvalidateRect(hwnd,0,0);
			return 0;
		}
		case WM_LBUTTONUP:{
			pan = false;
			return 0;
		}
		case WM_CHAR:{
			return 0;
		}
		case WM_KEYDOWN:{
			if (!(HIWORD(lParam) & KF_REPEAT)) switch (wParam){
				
			}
			return 0;
		}
		case WM_MOUSEWHEEL:{
			int oldScale = scale;
			scale = max(1,scale+CLAMP(GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA,-1,1));
			if (oldScale != scale){
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(hwnd,&pt);
				pt.y = clientHeight - pt.y - 1;
				pt.x -= pos.x;
				pt.y -= pos.y;
				if (scale > oldScale){
					pos.x -= pt.x / (float)(scale-1);
					pos.y -= pt.y / (float)(scale-1);
				} else {
					pos.x += pt.x / (float)(scale+1);
					pos.y += pt.y / (float)(scale+1);
				}
				InvalidateRect(hwnd,0,0);
				if (GetCursor() != cursorFinger) SetCursor(scale == 1 ? cursorArrow : cursorPan);
			}
			return 0;
		}
		case WM_PAINT:{
			glCheckError();

			glClearColor(0.122f,0.122f,0.137f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (texture.id){
				glUseProgram(TextureShader.id);
				glUniform1i(TextureShader.uTex,0);
				glBindTexture(GL_TEXTURE_2D,texture.id);
				float width = min(clientWidth,texture.width);
				float height = width * ((float)texture.height/(float)texture.width);
				if (height > workspaceHeight){
					height = workspaceHeight;
					width = height * ((float)texture.width/(float)texture.height);
				}
				width *= scale;
				height *= scale;
				if (scale == 1) pos = (FVec3){(clientWidth-(int)width)/2,(workspaceHeight-(int)height)/2,-1};
				glUniformMatrix4fv(TextureShader.uProj,1,GL_FALSE,mat4Mul(ortho,mat4Mul(mat4Pos(pos),mat4Scale((FVec3){width,height,1}))).arr);
				glBindBuffer(GL_ARRAY_BUFFER,imageQuad.vertexBuffer);
				TextureShaderPrepBuffer();
				glDrawArrays(GL_TRIANGLES,0,imageQuad.vertexCount);

				ListHeader *verts;
				glUseProgram(RoundedRectangleShader.id);
				glUniformMatrix4fv(RoundedRectangleShader.proj,1,GL_FALSE,ortho.arr);
				ListInit(&verts,sizeof(RoundedRectangleVertex),0);
				AppendRoundedRectangle(&verts,20,workspaceHeight/2,0,10,25,10,RGBA(238,84,12,RR_DISH),RGBA(255,255,255,RR_ICON_REVERSE_PLAY));
				AppendRoundedRectangle(&verts,clientWidth-20,workspaceHeight/2,0,10,25,10,RGBA(238,84,12,RR_DISH),RGBA(255,255,255,RR_ICON_PLAY));
				AppendRoundedRectangle(&verts,4+50,clientHeight-16,0,50,10,10,RGBA(127,127,127,RR_DISH),RGBA(0,0,0,RR_ICON_NONE));
				DrawTriangles(verts,RoundedRectangleShaderPrepBuffer,false);
				free(verts);

				glUseProgram(ColorShader.id);
				glUniformMatrix4fv(ColorShader.proj,1,GL_FALSE,ortho.arr);
				ListInit(&verts,sizeof(ColorVertex),0);
				AppendFormatStringMesh(&verts,&font,10,clientHeight-20,1,12,RGBA(255,255,255,255),L"Interpolation: %s",interpolation ? L"On" : L"Off");
				AppendFormatStringMesh(&verts,&font,120,clientHeight-20,1,12,RGBA(255,255,255,255),L"Scale: %d",scale);
				AppendFormatStringMesh(&verts,&font,180,clientHeight-20,1,12,RGBA(255,255,255,255),L"Image %d of %d in folder.",imageIndex+1,images->used);
				DrawTriangles(verts,ColorShaderPrepBuffer,false);
				free(verts);
			}

			SwapBuffers(hdc);
			ValidateRect(hwnd,0);
			return 0;
		}
	}
	return DefWindowProcW(hwnd,uMsg,wParam,lParam);
}
#if _DEBUG
void main(){
#else
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
#endif
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	DarkGLMakeWindow(RID_ICON,L"DarkViewer",clientWidth,clientHeight,WindowProc); //loads OpenGL functions

	ttf_t **list = ttf_list_system_fonts("segoeui*");
	if (!list) FatalErrorA("Out of memory");
	if (!list[0]) FatalErrorA("No requested fonts found");
	ttf_load_from_file(list[0]->filename,&font.ttf,false);
	ttf_free_list(list);
	GenFontMeshes(&font);

	GetModuleFileNameW(0,exePath,COUNT(exePath));
	int argc;
	WCHAR **argv = CommandLineToArgvW(GetCommandLineW(),&argc);
	if (argc==2) LoadImg(argv[1]);

	glGenBuffers(1,&imageQuad.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER,imageQuad.vertexBuffer);
	TextureVertex v[6] = {
		{{0,1,0},{0,1}},
		{{0,0,0},{0,0}},
		{{1,0,0},{1,0}},
		{{1,0,0},{1,0}},
		{{1,1,0},{1,1}},
		{{0,1,0},{0,1}}
	};
	glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
	imageQuad.vertexCount = 6;

	cursorArrow = LoadCursorA(0,IDC_ARROW);
	cursorFinger = LoadCursorA(0,IDC_HAND);
	cursorPan = LoadCursorA(0,IDC_SIZEALL);
	SetCursor(cursorArrow);

	ShowWindow(gwnd,SW_SHOWDEFAULT);
	MSG msg;
	while (GetMessageW(&msg,0,0,0)){
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}