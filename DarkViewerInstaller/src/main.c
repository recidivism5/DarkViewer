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
bool installationComplete = false;

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
		if (!installationComplete && PointInButton(clientWidth/2,clientHeight/2,120,25,x,y)) SetCursor(cursorFinger);
		else SetCursor(cursorArrow);
		return 0;
	}
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:{
		int x = GET_X_LPARAM(lParam);
		int y = clientHeight - GET_Y_LPARAM(lParam) - 1;
		if (!installationComplete && PointInButton(clientWidth/2,clientHeight/2,120,25,x,y)){
			SetCursor(cursorArrow);

			wprintf(L"Installing to %s\n",installFolder);
			CreateDirectoryW(installFolder,0);

			WCHAR path[512];
			_snwprintf(path,COUNT(path),L"%s\\DarkViewer.exe",installFolder);
			CopyResource(RID_DARKVIEWER,path);
			_snwprintf(path,COUNT(path),L"%s\\DarkViewerUninstaller.exe",installFolder);
			CopyResource(RID_DARKVIEWERUNINSTALLER,path);

			HKEY key;
			RegCreateKeyW(HKEY_CURRENT_USER,L"software\\microsoft\\windows\\currentversion\\uninstall\\DarkViewer",&key);
			RegSetValueExW(key,L"DisplayName",0,REG_SZ,L"DarkViewer",sizeof(L"DarkViewer"));
			RegSetValueExW(key,L"DisplayVersion",0,REG_SZ,L"1",sizeof(L"1"));
			RegSetValueExW(key,L"Publisher",0,REG_SZ,L"Dark Software",sizeof(L"Dark Software"));
			RegSetValueExW(key,L"UninstallString",0,REG_SZ,path,wcslen(path)*2+2);
			SHGetSpecialFolderPathW(0,path,CSIDL_LOCAL_APPDATA,0);
			wcscat(path,L"\\DarkViewer");
			RegSetValueExW(key,L"InstallLocation",0,REG_SZ,path,wcslen(path)*2+2);
			wcscat(path,L"\\DarkViewer.exe");
			RegSetValueExW(key,L"DisplayIcon",0,REG_SZ,path,wcslen(path)*2+2);
			DWORD d = 200;
			RegSetValueExW(key,L"EstimatedSize",0,REG_DWORD,&d,sizeof(d));
			RegCloseKey(key);

			IShellLinkW* psl;
			CoCreateInstance(&CLSID_ShellLink,0,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,&psl);
			psl->lpVtbl->SetPath(psl,path);
			psl->lpVtbl->SetDescription(psl,L"DarkViewer");
			IPersistFile* ppf;
			psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,&ppf);
			WCHAR *prgs;
			SHGetKnownFolderPath(&FOLDERID_Programs,0,0,&prgs);
			_snwprintf(path,COUNT(path),L"%s%s",prgs,L"\\DarkViewer.lnk");
			ppf->lpVtbl->Save(ppf,path,1);
			ppf->lpVtbl->Release(ppf);
			psl->lpVtbl->Release(psl);
			CoTaskMemFree(prgs);

			const wchar_t *extensions[]={
				L"png",
				L"jpg"
			};
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
							if ((size==sizeof(L"DarkViewer.exe")) && !_wcsnicmp(path,L"DarkViewer.exe",size/2)) goto NOCREATE;
						} else {
							FatalErrorW(L".%s Explorer FileExts registry key is corrupted.",extensions[i]);
						}
						c[0]++;
						if (c[0]>'z') goto NEXT;
					}
					RegSetValueExW(key,c,0,REG_SZ,L"DarkViewer.exe",sizeof(L"DarkViewer.exe"));
				NOCREATE:
					size = sizeof(path);
					int count;
					if (ERROR_SUCCESS == RegQueryValueExW(key,L"mrulist",0,&type,path,&size)) count = size/2;
					else count = 0;
					for (int j = 0; j < count; j++){
						if (path[j]==c[0]) goto NEXT;
					}
					if (count && !path[count-1]){
						path[count-1] = c[0];
						path[count] = 0;
						count += 1;
					} else {
						path[count] = c[0];
						path[count+1] = 0;
						count += 2;
					}
					RegSetValueExW(key,L"mrulist",0,REG_SZ,path,count*2);
				NEXT:
					RegCloseKey(key);
				}
			}

			installationComplete = true;
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
		if (!installationComplete){
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
		AppendFormatCenteredStringMesh(&verts,&font,clientWidth/2,clientHeight/2+100,1,24,RGBA(255,255,255,255),L"Will install to %s",installFolder);
		AppendFormatCenteredStringMesh(&verts,&font,clientWidth/2,clientHeight/2+60,1,24,RGBA(255,255,255,255),L"To uninstall use \"Add or remove programs\".");
		AppendFormatCenteredStringMesh(&verts,&font,clientWidth/2,clientHeight/2,1,24,RGBA(255,255,255,255),installationComplete ? L"Done." : L"Install DarkViewer");
		if (installationComplete) AppendFormatCenteredStringMesh(&verts,&font,clientWidth/2,clientHeight/2-60,1,24,RGBA(255,255,255,255),L"It is safe to close this installer.");
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