#include "../../DarkEngine/src/DarkEngine.h"
#include "res.h"

HINSTANCE hinstance;
WCHAR installFolder[MAX_PATH];
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
bool uninstallationComplete = false;

bool PointInButton(int buttonX, int buttonY, int halfWidth, int halfHeight, int x, int y){
	return abs(x-buttonX) < halfWidth && abs(y-buttonY) < halfHeight;
}

void CopyResource(int resourceId, WCHAR *path){
	HANDLE hfile = CreateFileW(path,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	HRSRC res = FindResourceW(hinstance,MAKEINTRESOURCEW(resourceId),RT_RCDATA);
	char *data = LockResource(LoadResource(0,res));
	size_t size = SizeofResource(0,res);
	WriteFile(hfile,data,size,0,0);
	CloseHandle(hfile);
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_CREATE:{
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
		if (!uninstallationComplete && PointInButton(clientWidth/2,clientHeight/2,120,25,x,y)) SetCursor(cursorFinger);
		else SetCursor(cursorArrow);
		return 0;
	}
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:{
		int x = GET_X_LPARAM(lParam);
		int y = clientHeight - GET_Y_LPARAM(lParam) - 1;
		if (!uninstallationComplete && PointInButton(clientWidth/2,clientHeight/2,120,25,x,y)){
			SetCursor(cursorArrow);

			WCHAR path[512];
			_snwprintf(path,COUNT(path),L"%s\\DarkViewer.exe",installFolder);
			DeleteFileW(path);
			_snwprintf(path,COUNT(path),L"%s\\DarkViewerUninstaller.exe",installFolder);
			MoveFileExW(path,0,MOVEFILE_DELAY_UNTIL_REBOOT);//will delete on startup
			MoveFileExW(installFolder,0,MOVEFILE_DELAY_UNTIL_REBOOT);

			RegDeleteKeyW(HKEY_CURRENT_USER,L"software\\microsoft\\windows\\currentversion\\uninstall\\DarkViewer");

			WCHAR *prgs;
			SHGetKnownFolderPath(&FOLDERID_Programs,0,0,&prgs);
			_snwprintf(path,COUNT(path),L"%s%s",prgs,L"\\DarkViewer.lnk");
			DeleteFileW(path);

			const wchar_t *extensions[]={
				L"png",
				L"jpg"
			};
			HKEY key;
			for (int i = 0; i < COUNT(extensions); i++){
				_snwprintf(path,COUNT(path),L"software\\microsoft\\windows\\currentversion\\explorer\\fileexts\\.%s\\openwithlist",extensions[i]);
				if (ERROR_SUCCESS == RegOpenKeyW(HKEY_CURRENT_USER,path,&key)){
					WCHAR c[2] = L"a";
					DWORD type;
					DWORD size;
					while (1){
						size = sizeof(path);
						if (ERROR_SUCCESS != RegQueryValueExW(key,c,0,&type,path,&size)) break;
						if (type==REG_SZ && size){
							if ((size==sizeof(L"DarkViewer.exe")) && !_wcsnicmp(path,L"DarkViewer.exe",size/2)){
								RegDeleteValueW(key,c);
								size = sizeof(path);
								int count;
								if (ERROR_SUCCESS == RegQueryValueExW(key,L"mrulist",0,&type,path,&size)){
									count = size/2;
									for (int j = 0; j < count; j++){
										if (path[j]==c[0]){
											memmove(path+j,path+j+1,(count-1-j)*sizeof(WCHAR));
											count--;
											if (path[count-1]){
												path[count] = 0;
												count++;
											}
											RegSetValueExW(key,L"mrulist",0,REG_SZ,path,count*2);
										}
									}
								}
							}
						} else {
							FatalErrorW(L".%s Explorer FileExts registry key is corrupted.",extensions[i]);
						}
						c[0]++;
						if (c[0]>'z') break;
					}
					RegCloseKey(key);
				}
			}

			uninstallationComplete = true;
			InvalidateRect(hwnd,0,0);
		}
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
	case WM_PAINT:{
		glCheckError();

		glClearColor(0.122f,0.122f,0.137f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ListHeader *verts;
		if (!uninstallationComplete){
			glUseProgram(RoundedRectangleShader.id);
			glUniformMatrix4fv(RoundedRectangleShader.proj,1,GL_FALSE,ortho.arr);
			ListInit(&verts,sizeof(RoundedRectangleVertex),0);
			AppendRoundedRectangle(&verts,clientWidth/2,clientHeight/2,0,120,25,25,RGBA(238,84,12,RR_DISH),RGBA(0,0,0,RR_ICON_NONE));
			DrawTriangles(verts,RoundedRectangleShaderPrepBuffer,false);
			free(verts);
		}

		glUseProgram(ColorShader.id);
		glUniformMatrix4fv(ColorShader.proj,1,GL_FALSE,ortho.arr);
		ListInit(&verts,sizeof(ColorVertex),0);
		AppendFormatCenteredStringMesh(&verts,&font,clientWidth/2,clientHeight/2+60,1,24,RGBA(255,255,255,255),L"Will uninstall from %s",installFolder);
		AppendFormatCenteredStringMesh(&verts,&font,clientWidth/2,clientHeight/2,1,24,RGBA(255,255,255,255),uninstallationComplete ? L"Done." : L"Uninstall DarkViewer");
		if (uninstallationComplete) AppendFormatCenteredStringMesh(&verts,&font,clientWidth/2,clientHeight/2-60,1,24,RGBA(255,255,255,255),L"It is safe to close this uninstaller.");
		DrawTriangles(verts,ColorShaderPrepBuffer,false);
		free(verts);

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
	hinstance = GetModuleHandleW(0);
	CoInitialize(0);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	DarkGLMakeWindow(RID_ICON,L"DarkViewer",clientWidth,clientHeight,WindowProc); //loads OpenGL functions

	ttf_t **list = ttf_list_system_fonts("segoeui*");
	if (!list) FatalErrorA("Out of memory");
	if (!list[0]) FatalErrorA("No requested fonts found");
	ttf_load_from_file(list[0]->filename,&font.ttf,false);
	ttf_free_list(list);
	GenFontMeshes(&font);

	SHGetSpecialFolderPathW(0,installFolder,CSIDL_LOCAL_APPDATA,0);
	wcscat(installFolder,L"\\DarkViewer");

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

	CoUninitialize();
}