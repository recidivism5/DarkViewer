#include "DarkEngine.h"

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
#define GL_FUNCTIONS(X) \
	X(PFNGLCREATESHADERPROC,             glCreateShader             ) \
	X(PFNGLSHADERSOURCEPROC,             glShaderSource             ) \
	X(PFNGLGETSHADERIVPROC,              glGetShaderiv              ) \
	X(PFNGLGETSHADERINFOLOGPROC,         glGetShaderInfoLog         ) \
	X(PFNGLGETPROGRAMIVPROC,             glGetProgramiv             ) \
	X(PFNGLGETPROGRAMINFOLOGPROC,        glGetProgramInfoLog        ) \
	X(PFNGLCOMPILESHADERPROC,            glCompileShader            ) \
	X(PFNGLCREATEPROGRAMPROC,            glCreateProgram            ) \
	X(PFNGLATTACHSHADERPROC,             glAttachShader             ) \
	X(PFNGLLINKPROGRAMPROC,              glLinkProgram              ) \
	X(PFNGLDELETESHADERPROC,             glDeleteShader             ) \
	X(PFNGLUSEPROGRAMPROC,				 glUseProgram				) \
	X(PFNGLGETATTRIBLOCATIONPROC,        glGetAttribLocation        ) \
	X(PFNGLGETUNIFORMLOCATIONPROC,       glGetUniformLocation       ) \
	X(PFNGLGENVERTEXARRAYSPROC,          glGenVertexArrays          ) \
	X(PFNGLBINDVERTEXARRAYPROC,          glBindVertexArray          ) \
	X(PFNGLGENBUFFERSPROC,               glGenBuffers               ) \
	X(PFNGLBINDBUFFERPROC,               glBindBuffer               ) \
	X(PFNGLBUFFERDATAPROC,               glBufferData               ) \
	X(PFNGLDELETEBUFFERSPROC,            glDeleteBuffers            ) \
	X(PFNGLENABLEVERTEXATTRIBARRAYPROC,  glEnableVertexAttribArray  ) \
	X(PFNGLVERTEXATTRIBPOINTERPROC,      glVertexAttribPointer      ) \
	X(PFNGLUNIFORM1FPROC,                glUniform1f                ) \
	X(PFNGLUNIFORM2FPROC,                glUniform2f                ) \
	X(PFNGLUNIFORM3FPROC,                glUniform3f                ) \
	X(PFNGLUNIFORM4FPROC,                glUniform4f                ) \
	X(PFNGLUNIFORM1FVPROC,               glUniform1fv               ) \
	X(PFNGLUNIFORM2FVPROC,               glUniform2fv               ) \
	X(PFNGLUNIFORM3FVPROC,               glUniform3fv               ) \
	X(PFNGLUNIFORM4FVPROC,               glUniform4fv               ) \
	X(PFNGLUNIFORM1IPROC,                glUniform1i                ) \
	X(PFNGLUNIFORM2IPROC,                glUniform2i                ) \
	X(PFNGLUNIFORM3IPROC,                glUniform3i                ) \
	X(PFNGLUNIFORM4IPROC,                glUniform4i                ) \
	X(PFNGLUNIFORM1IVPROC,               glUniform1iv               ) \
	X(PFNGLUNIFORM2IVPROC,               glUniform2iv               ) \
	X(PFNGLUNIFORM3IVPROC,               glUniform3iv               ) \
	X(PFNGLUNIFORM4IVPROC,               glUniform4iv               ) \
	X(PFNGLUNIFORMMATRIX4FVPROC,         glUniformMatrix4fv         ) \
	X(PFNGLDEBUGMESSAGECALLBACKPROC,     glDebugMessageCallback     )
#define X(type, name) type name;
GL_FUNCTIONS(X)
#undef X

void FatalErrorA(char *format, ...){
	va_list args;
	va_start(args,format);
#if _DEBUG
	vprintf(format,args);
	printf("\n");
#else
	char msg[512];
	vsprintf(msg,format,args);
	MessageBoxA(0,msg,"Error",MB_ICONEXCLAMATION);
#endif
	va_end(args);
	exit(1);
}
void FatalErrorW(WCHAR *format, ...){
	va_list args;
	va_start(args,format);
#if _DEBUG
	vwprintf(format,args);
	wprintf(L"\n");
#else
	WCHAR msg[512];
	vswprintf(msg,COUNT(msg),format,args);
	MessageBoxW(0,msg,L"Error",MB_ICONEXCLAMATION);
#endif
	va_end(args);
	exit(1);
}

void *MallocOrDie(size_t size){
	void *p = malloc(size);
	if (!p) FatalErrorA("malloc failed.");
	return p;
}
void *ZallocOrDie(size_t size){
	void *p = calloc(1,size);
	if (!p) FatalErrorA("calloc failed.");
	return p;
}
void *ReallocOrDie(void *ptr, size_t size){
	void *p = realloc(ptr,size);
	if (!p) FatalErrorA("realloc failed.");
	return p;
}

/*
randint
From: https://stackoverflow.com/a/822361
Generates uniform random integers in range [0,n).
*/
int randint(int n){
	if ((n - 1) == RAND_MAX){
		return rand();
	} else {
		// Chop off all of the values that would cause skew...
		int end = RAND_MAX / n; // truncate skew
		end *= n;
		// ... and ignore results from rand() that fall above that limit.
		// (Worst case the loop condition should succeed 50% of the time,
		// so we can expect to bail out of this loop pretty quickly.)
		int r;
		while ((r = rand()) >= end);
		return r % n;
	}
}
/******************* Files */
char *LoadFileA(char *path, size_t *size){
	FILE *f = fopen(path,"rb");
	if (!f) FatalErrorA("File not found: %s",path);
	fseek(f,0,SEEK_END);
	*size = ftell(f);
	fseek(f,0,SEEK_SET);
	char *buf = MallocOrDie(*size);
	fread(buf,*size,1,f);
	fclose(f);
	return buf;
}
char *LoadFileW(WCHAR *path, size_t *size){
	FILE *f = _wfopen(path,L"rb");
	if (!f) FatalErrorW(L"File not found: %s",path);
	fseek(f,0,SEEK_END);
	*size = ftell(f);
	fseek(f,0,SEEK_SET);
	char *buf = MallocOrDie(*size);
	fread(buf,*size,1,f);
	fclose(f);
	return buf;
}
int CharIsSuitableForFileName(char c){
	return (c==' ') || ((','<=c)&&(c<='9')) || (('A'<=c)&&(c<='Z')) || (('_'<=c)&&(c<='z'));
}
ByteArray ByteArrayFromFileA(char *path){
	ByteArray ba;
	ba.bytes = LoadFileA(path,&ba.size);
	return ba;
}
ByteArray ByteArrayFromFileW(WCHAR *path){
	ByteArray ba;
	ba.bytes = LoadFileW(path,&ba.size);
	return ba;
}
BOOL FileOrFolderExists(LPCTSTR path){
	return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

/******************* List */
void ListInit(ListHeader **listp, int elementSize, int total){//allocates a new list with total elements, zeroed
	*listp = ZallocOrDie(sizeof(ListHeader)+total*elementSize);
	(*listp)->elementSize = elementSize;
	(*listp)->total = total;
	(*listp)->used = 0;
}
int ListIndexOf(ListHeader* list, void* element){
	return ((char *)element - (char *)(list+1)) / list->elementSize;
}
void *ListGet(ListHeader *list, int index){
	return ((char *)(list+1))+index*list->elementSize;
}
void ListSet(ListHeader *list, int index, void *elements, int count){
	memcpy(ListGet(list,index),elements,count*list->elementSize);
}
void ListInsert(ListHeader **listp, int index, void *elements, int count){
	ListHeader *list = *listp;
	if (list->total-list->used < count){
		if (!list->total) list->total = 1;
		while (list->total-list->used < count) list->total *= 2;
		list = ReallocOrDie(list,sizeof(ListHeader)+list->total*list->elementSize);
	}
	if (index != list->used) memmove(((char *)(list+1))+(index+count)*list->elementSize,((char *)(list+1))+index*list->elementSize,(list->used-index)*list->elementSize);
	memcpy(((char *)(list+1))+index*list->elementSize,elements,count*list->elementSize);
	list->used += count;
	*listp = list;
}
void ListRemove(ListHeader **listp, int index, int count){
	ListHeader *list = *listp;
	memmove(((char *)(list+1))+index*list->elementSize,((char *)(list+1))+(index+count)*list->elementSize,(list->used-(index+count))*list->elementSize);
	list->used -= count;
	*listp = list;
}
void ListAppend(ListHeader **listp, void *elements, int count){
	ListInsert(listp,(*listp)->used,elements,count);
}
int Find(int *indices, int index){
	while (index != indices[index]) index = indices[index];
	return index;
}
void Union(int *indices, int aIndex, int bIndex){
	int aRoot = Find(indices,aIndex);
	while (aIndex != indices[aIndex]){
		int ni = indices[aIndex];
		indices[aIndex] = aRoot;
		aIndex = ni;
	}
	while (bIndex != indices[bIndex]){
		int ni = indices[bIndex];
		indices[bIndex] = aRoot;
		bIndex = ni;
	}
	indices[bIndex] = aRoot;
}

/***************************** HashList
Based on https://craftinginterpreters.com/hash-tables.html
*/
uint32_t fnv1a(uint8_t *key, int len){
	uint32_t hash = 2166136261u;
	for (int i = 0; i < len; i++){
		hash ^= key[i];
		hash *= 16777619;
	}
	return hash;
}
void HashListInit(ListHeader **listp, size_t elementSize){
	ListInit(listp,elementSize,HASHLIST_STARTING_TOTAL);
}
BucketHeader *HashListGet(ListHeader *list, char *key, int keylen){
	uint32_t index = fnv1a(key,keylen) % list->total;
	BucketHeader *lastTombstone = 0;
	while (1){
		BucketHeader *b = ListGet(list,index);
		if (b->keylen == HASHLIST_TOMBSTONE) lastTombstone = b;
		else if (!b->keylen) return lastTombstone ? lastTombstone : b;
		else if (b->keylen == keylen && !memcmp(b->key,key,keylen)) return b;
		index = (index+1) % list->total;
	}
}
void HashListSet(ListHeader **listp, BucketHeader *bucket){
	ListHeader *list = *listp;
	if (list->used+1 > list->total*HASHLIST_MAX_LOAD){
		ListHeader *newList;
		ListInit(&newList,list->elementSize,list->total*2);
		for (int i = 0; i < list->total; i++){
			BucketHeader *b = ListGet(list,i);
			if (b->keylen && b->keylen != HASHLIST_TOMBSTONE){
				memcpy(HashListGet(newList,b->key,b->keylen),b,list->elementSize);
			}
		}
		free(list);
		list = newList;
	}
	BucketHeader *b = HashListGet(list,bucket->key,bucket->keylen);
	if (!b->keylen || b->keylen == HASHLIST_TOMBSTONE) list->used++;
	memcpy(b,bucket,list->elementSize);
	*listp = list;
}
int HashListRemove(ListHeader **listp, char *key, int keylen){
	BucketHeader *b = HashListGet(*listp,key,keylen);
	if (b->keylen && b->keylen != HASHLIST_TOMBSTONE){
		b->keylen = HASHLIST_TOMBSTONE;
		(*listp)->used--;
		return 1;
	}
	return 0;
}

/***************************** RingBuffer */
void RingBufferReset(RingBuffer *r){
	r->used = 0;
	r->writeIndex = 0;
	r->readIndex = 0;
}
void RingBufferInit(RingBuffer *r, size_t elementSize, int total, void *elements){
	r->elementSize = elementSize;
	r->total = total;
	r->elements = elements;
	RingBufferReset(r);
}
void* RingBufferGet(RingBuffer *r, int index){
	return ((char *)r->elements)+((r->readIndex+index)%r->total)*r->elementSize;
}
void *RingBufferIncrementWriteHead(RingBuffer *r){
	if (r->used == r->total) return 0;
	void *w = ((char *)r->elements)+(r->writeIndex%r->total)*r->elementSize;
	r->writeIndex = (r->writeIndex + 1) % r->total;
	r->used++;
	return w;
}
void RingBufferIncrementReadHead(RingBuffer *r){
	if (!r->used) return;
	r->readIndex = (r->readIndex + 1) % r->total;
	r->used--;
}

/***************************** ulinalg */
float Lerp(float a, float b, float t){
	return a + (b-a) * t;
}
float fvec3Dot(FVec3 a, FVec3 b){
	return a.x*b.x + a.y*b.y + a.z*b.z;
}
float fvec3Length(FVec3 v){
	return sqrtf(fvec3Dot(v,v));
}
FVec3 fvec3Cross(FVec3 a, FVec3 b){
	return (FVec3){
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x
	};
}
FVec3 fvec3Add(FVec3 a, FVec3 b){
	return (FVec3){a.x+b.x,a.y+b.y,a.z+b.z};
}
FVec3 fvec3Sub(FVec3 a, FVec3 b){
	return (FVec3){a.x-b.x,a.y-b.y,a.z-b.z};
}
FVec3 fvec3Negate(FVec3 a){
	return (FVec3){-a.x,-a.y,-a.z};
}
FVec3 fvec3Scale(FVec3 v, float s){
	return (FVec3){v.x*s,v.y*s,v.z*s};
}
FVec3 fvec3Norm(FVec3 v){
	return fvec3Scale(v, 1.0f/fvec3Length(v));
}
FVec3 fvec3SetLength(FVec3 v, float length){
	return fvec3Scale(fvec3Norm(v), length);
}
FVec3 fvec3Midpoint(FVec3 a, FVec3 b){
	return fvec3Add(fvec3Scale(fvec3Sub(b,a),0.5f),a);
}
FVec3 fvec3Lerp(FVec3 a, FVec3 b, float t){
	return (FVec3){Lerp(a.x,b.x,t),Lerp(a.y,b.y,t),Lerp(a.z,b.z,t)};
}
FVec4 fvec4Lerp(FVec4 a, FVec4 b, float t){
	return (FVec4){Lerp(a.x,b.x,t),Lerp(a.y,b.y,t),Lerp(a.z,b.z,t),Lerp(a.w,b.w,t)};
}
float fvec3Dist(FVec3 a, FVec3 b){
	return fvec3Length(fvec3Sub(a,b));
}
float fvec3AngleBetween(FVec3 a, FVec3 b){
	return acosf(fvec3Dot(a,b)/(fvec3Length(a)*fvec3Length(b)));
}
FVec3 clampEuler(FVec3 e){
	float fp = 4*M_PI;
	for (int i = 0; i < 3; i++){
		if (e.arr[i] > fp) e.arr[i] -= fp;
		else if (e.arr[i] < -fp) e.arr[i] += fp;
	}
	return e;
}
float fvec4Dot(FVec4 a, FVec4 b){
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}
FVec4 quatSlerp(FVec4 a, FVec4 b, float t){//from https://github.com/microsoft/referencesource/blob/51cf7850defa8a17d815b4700b67116e3fa283c2/System.Numerics/System/Numerics/Quaternion.cs#L289C8-L289C8
	float epsilon = 1e-6f;
	float cosOmega = fvec4Dot(a,b);
	bool flip = false;
	if (cosOmega < 0.0f){
		flip = true;
		cosOmega = -cosOmega;
	}
	float s1,s2;
	if (cosOmega > (1.0f - epsilon)){
		s1 = 1.0f - t;
		s2 = flip ? -t : t;
	} else {
		float omega = acosf(cosOmega);
		float invSinOmega = 1.0f / sinf(omega);
		s1 = sinf((1.0f-t)*omega)*invSinOmega;
		float sm = sinf(t*omega)*invSinOmega;
		s2 = flip ? -sm : sm;
	}
	return (FVec4){
		s1*a.x+s2*b.x,
		s1*a.y+s2*b.y,
		s1*a.z+s2*b.z,
		s1*a.w+s2*b.w
	};
}
Mat4 mat4Identity(void){
	return (Mat4){
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
}
Mat4 mat4Basis(FVec3 x, FVec3 y, FVec3 z){
	Mat4 m = mat4Identity();
	m.a.vec3 = x;
	m.b.vec3 = y;
	m.c.vec3 = z;
	return m;
}
Mat4 mat4Transpose(Mat4 m){
	float t;
	SWAP(t,m.a1,m.b0);
	SWAP(t,m.a2,m.c0);
	SWAP(t,m.a3,m.d0);
	SWAP(t,m.b2,m.c1);
	SWAP(t,m.b3,m.d1);
	SWAP(t,m.c3,m.d2);
	return m;
}
Mat4 mat4TransposeMat3(Mat4 m){
	/*
	a0 b0 c0 d0
	a1 b1 c1 d1
	a2 b2 c2 d2
	a3 b3 c3 d3
	*/
	float t;
	SWAP(t,m.a1,m.b0);
	SWAP(t,m.a2,m.c0);
	SWAP(t,m.b2,m.c1);
	return m;
}
Mat4 mat4Ortho(float left, float right, float bottom, float top, float near, float far){
	return (Mat4){
		2/(right-left),0,0,0,
		0,2/(top-bottom),0,0,
		0,0,2/(near-far),0,
		(right+left)/(left-right),(top+bottom)/(bottom-top),(far+near)/(near-far),1
	};
}
Mat4 mat4Persp(float fovRadians, float aspectRatio, float near, float far){
	float s = 1.0f / tanf(fovRadians * 0.5f);
	float d = near - far;
	return (Mat4){
		s/aspectRatio,0,0,0,
		0,s,0,0,
		0,0,(far+near)/d,-1,
		0,0,(2*far*near)/d,0
	};
}
Mat4 mat4Scale(FVec3 v){
	Mat4 m = mat4Identity();
	m.a0 = v.x;
	m.b1 = v.y;
	m.c2 = v.z;
	return m;
}
Mat4 mat4Pos(FVec3 v){
	Mat4 m = mat4Identity();
	m.d.vec3 = v;
	return m;
}
Mat4 mat4RotX(float angle){
	float c = cos(angle), s = sin(angle);
	return (Mat4){
		1,0,0,0,
		0,c,s,0,
		0,-s,c,0,
		0,0,0,1
	};
}
Mat4 mat4RotY(float angle){
	float c = cos(angle), s = sin(angle);
	return (Mat4){
		c,0,-s,0,
		0,1,0,0,
		s,0,c,0,
		0,0,0,1
	};
}
Mat4 mat4RotZ(float angle){
	float c = cos(angle), s = sin(angle);
	return (Mat4){
		c,s,0,0,
		-s,c,0,0,
		0,0,1,0,
		0,0,0,1
	};
}
Mat4 mat4Mul(Mat4 a, Mat4 b){
	Mat4 m;
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m.arr[i*4+j] = a.arr[j]*b.arr[i*4] + a.arr[j+4]*b.arr[i*4+1] + a.arr[j+8]*b.arr[i*4+2] + a.arr[j+12]*b.arr[i*4+3];
	return m;
}
FVec4 mat4MulFVec4(Mat4 m, FVec4 v){
	FVec4 r;
	for (int i = 0; i < 4; i++) r.arr[i] = v.x*m.a.arr[i] + v.y*m.b.arr[i] + v.z*m.c.arr[i] + v.w*m.d.arr[i];
	return r;
}
Mat4 mat4LookAt(FVec3 eye, FVec3 target){
	FVec3 z = fvec3Norm(fvec3Sub(eye,target)),
		x = fvec3Norm(fvec3Cross(z,(FVec3){0,1,0})),
		y = fvec3Cross(x,z);
	return mat4Mul(mat4Transpose(mat4Basis(fvec3Negate(x),y,z)),mat4Pos(fvec3Negate(eye)));
}
Mat4 mat4LookAtShake(FVec3 eye, FVec3 target, FVec3 shakeEuler){
	FVec3 z = fvec3Norm(fvec3Sub(eye,target)),
		x = fvec3Norm(fvec3Cross(z,(FVec3){0,1,0})),
		y = fvec3Cross(x,z);
	return mat4Mul(mat4Transpose(mat4Mul(eulerToMat4(shakeEuler),mat4Basis(fvec3Negate(x),y,z))),mat4Pos(fvec3Negate(eye)));
}
Mat4 eulerToMat4(FVec3 e){
	return mat4Mul(mat4RotZ(e.z),mat4Mul(mat4RotY(e.y),mat4RotX(e.x)));
}
Mat4 quatToMat4(FVec4 q){
	Mat4 m = mat4Identity();
	m.a0 = 1 - 2*(q.y*q.y + q.z*q.z);
	m.a1 = 2*(q.x*q.y + q.z*q.w);
	m.a2 = 2*(q.x*q.z - q.y*q.w);

	m.b0 = 2*(q.x*q.y - q.z*q.w);
	m.b1 = 1 - 2*(q.x*q.x + q.z*q.z);
	m.b2 = 2*(q.y*q.z + q.x*q.w);

	m.c0 = 2*(q.x*q.z + q.y*q.w);
	m.c1 = 2*(q.y*q.z - q.x*q.w);
	m.c2 = 1 - 2*(q.x*q.x + q.y*q.y);
	return m;
}
Mat4 mat4MtwInverse(Mat4 m){
	Mat4 i = mat4TransposeMat3(m);
	i.d.vec3 = fvec3Negate(m.d.vec3);
	return i;
}
void mat4Print(Mat4 m){
	for (int i = 0; i < 4; i++){
		printf("%f %f %f %f\n",m.arr[0*4+i],m.arr[1*4+i],m.arr[2*4+i],m.arr[3*4+i]);
	}
}
FVec3 fvec3Rotated(FVec3 v, FVec3 euler){
	return mat4MulFVec4(eulerToMat4(euler),(FVec4){v.x,v.y,v.z,0.0f}).vec3;
}
FVec3 normalFromTriangle(FVec3 a, FVec3 b, FVec3 c){
	return fvec3Norm(fvec3Cross(fvec3Sub(b,a),fvec3Sub(c,a)));
}
int manhattanDistance(IVec3 a, IVec3 b){
	return abs(a.x-b.x)+abs(a.y-b.y)+abs(a.z-b.z);
}
Mat4 getVP(Camera *c){
	return mat4Mul(mat4Persp(c->fov,c->aspect,0.01f,1024.0f),mat4Mul(mat4Transpose(eulerToMat4(c->euler)),mat4Pos(fvec3Scale(c->position,-1))));
}
void rotateCamera(Camera *c, float dx, float dy, float sens){
	c->euler.y += sens * dx;
	c->euler.x += sens * dy;
	c->euler = clampEuler(c->euler);
}
/************* OBB code from https://stackoverflow.com/questions/47866571/simple-oriented-bounding-box-obb-collision-detection-explaining#:~:text=To%20know%20if%20two%20OBB,normals%20there%20is%20a%20collision */
bool getSeparatingPlane(FVec3 RPos, FVec3 Plane, FVec3 *aNormals, FVec3 aHalfExtents, FVec3 *bNormals, FVec3 bHalfExtents){
	return (fabsf(fvec3Dot(RPos,Plane)) > 
			   (fabsf(fvec3Dot(fvec3Scale(aNormals[0],aHalfExtents.x),Plane)) +
				fabsf(fvec3Dot(fvec3Scale(aNormals[1],aHalfExtents.y),Plane)) +
				fabsf(fvec3Dot(fvec3Scale(aNormals[2],aHalfExtents.z),Plane)) +
				fabsf(fvec3Dot(fvec3Scale(bNormals[0],bHalfExtents.x),Plane)) + 
				fabsf(fvec3Dot(fvec3Scale(bNormals[1],bHalfExtents.y),Plane)) +
				fabsf(fvec3Dot(fvec3Scale(bNormals[2],bHalfExtents.z),Plane))));
}
bool OBBsColliding(OBB *a, OBB *b){
	FVec3 RPos = fvec3Sub(b->position,a->position);
	Mat4 aRot = eulerToMat4(a->euler);
	FVec3 aNormals[3] = {
		aRot.a.vec3,
		aRot.b.vec3,
		aRot.c.vec3
	};
	Mat4 bRot = eulerToMat4(b->euler);
	FVec3 bNormals[3] = {
		bRot.a.vec3,
		bRot.b.vec3,
		bRot.c.vec3
	};
	return !(getSeparatingPlane(RPos,aNormals[0],aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,aNormals[1],aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,aNormals[2],aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,bNormals[0],aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,bNormals[1],aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,bNormals[2],aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[0],bNormals[0]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[0],bNormals[1]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[0],bNormals[2]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[1],bNormals[0]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[1],bNormals[1]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[1],bNormals[2]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[2],bNormals[0]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[2],bNormals[1]),aNormals,a->halfExtents,bNormals,b->halfExtents) ||
			 getSeparatingPlane(RPos,fvec3Cross(aNormals[2],bNormals[2]),aNormals,a->halfExtents,bNormals,b->halfExtents));
}

/***************************** GLUtil */
GLenum glCheckError_(const char *file, int line){
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR){
		char *error;
		switch (errorCode){
		case GL_INVALID_ENUM:      error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:     error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:     error = "OUT_OF_MEMORY"; break;
		default: error = "UNKNOWN TYPE BEAT";break;
		}
		FatalErrorA("%s %s (%d)",error,file,line);
	}
	return errorCode;
}
void ImageFromFileA(Image *i, char *path){
	stbi_set_flip_vertically_on_load(1);
	ByteArray ba = ByteArrayFromFileA(path);
	i->pixels = stbi_load_from_memory(ba.bytes,ba.size,&i->width,&i->height,&i->comp,4);
	free(ba.bytes);
}
void ImageFromFileW(Image *i, WCHAR *path){
	stbi_set_flip_vertically_on_load(1);
	ByteArray ba = ByteArrayFromFileW(path);
	i->pixels = stbi_load_from_memory(ba.bytes,ba.size,&i->width,&i->height,&i->comp,4);
	free(ba.bytes);
}
void TextureFromImage(Texture *t, Image *i, bool interpolated){
	t->width = i->width;
	t->height = i->height;
	if (!t->id) glGenTextures(1,&t->id);
	glBindTexture(GL_TEXTURE_2D,t->id);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,interpolated ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,interpolated ? GL_LINEAR : GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,t->width,t->height,0,GL_RGBA,GL_UNSIGNED_BYTE,i->pixels);
}
void TextureFromFileA(Texture *t, char *path, bool interpolated){
	Image i;
	ImageFromFileA(&i,path);
	TextureFromImage(t,&i,interpolated);
	free(i.pixels);
}
void TextureFromFileW(Texture *t, WCHAR *path, bool interpolated){
	Image i;
	ImageFromFileW(&i,path);
	TextureFromImage(t,&i,interpolated);
	free(i.pixels);
}
void CheckShader(GLuint id){
	GLint result;
	glGetShaderiv(id,GL_COMPILE_STATUS,&result);
	if (!result){
		char infolog[512];
		glGetShaderInfoLog(id,512,NULL,infolog);
		FatalErrorA(infolog);
	}
}
void CheckProgram(GLuint id, GLenum param){
	GLint result;
	glGetProgramiv(id,param,&result);
	if (!result){
		char infolog[512];
		glGetProgramInfoLog(id,512,NULL,infolog);
		FatalErrorA(infolog);
	}
}
GLuint CompileShader(char *name, char *vertSrc, char *fragSrc){
	printf("Compiling %s\n",name);
	GLuint v = glCreateShader(GL_VERTEX_SHADER);
	GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(v,1,&vertSrc,NULL);
	glShaderSource(f,1,&fragSrc,NULL);
	glCompileShader(v);
	printf("\tvertex shader\n");
	CheckShader(v);
	glCompileShader(f);
	printf("\tfragment shader\n");
	CheckShader(f);
	GLuint p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);
	glLinkProgram(p);
	CheckProgram(p,GL_LINK_STATUS);
	glDeleteShader(v);
	glDeleteShader(f);
	return p;
}

/*********** Shaders */
void DrawTriangles(ListHeader *verts, void (*prepBuffer)(void), bool wireframe){
	GLuint buf;
	glGenBuffers(1,&buf);
	glBindBuffer(GL_ARRAY_BUFFER,buf);
	glBufferData(GL_ARRAY_BUFFER,verts->used*verts->elementSize,verts+1,GL_STATIC_DRAW);
	prepBuffer();
	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glDrawArrays(GL_TRIANGLES,0,verts->used);
	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glDeleteBuffers(1,&buf);
}
struct ColorShader ColorShader = {
	"#version 110\n"
	"attribute vec3 aPosition;\n"
	"attribute vec4 aColor;\n"
	"uniform mat4 proj;\n"
	"varying vec4 Color;\n"
	"void main(){\n"
	"	gl_Position = proj * vec4(aPosition,1.0);\n"
	"	Color = aColor;\n"
	"}",

	"#version 110\n"
	"varying vec4 Color;\n"
	"void main(){\n"
	"	gl_FragColor = Color;\n"
	"}"
};
static void CompileColorShader(){
	ColorShader.id = CompileShader("ColorShader",ColorShader.vertSrc,ColorShader.fragSrc);
	ColorShader.aPosition = glGetAttribLocation(ColorShader.id,"aPosition");
	ColorShader.aColor = glGetAttribLocation(ColorShader.id,"aColor");
	ColorShader.proj = glGetUniformLocation(ColorShader.id,"proj");
}
void ColorShaderPrepBuffer(){
	glEnableVertexAttribArray(ColorShader.aPosition);
	glEnableVertexAttribArray(ColorShader.aColor);
	glVertexAttribPointer(ColorShader.aPosition,3,GL_FLOAT,GL_FALSE,sizeof(ColorVertex),0);
	glVertexAttribPointer(ColorShader.aColor,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(ColorVertex),offsetof(ColorVertex,color));
}
void GenFontMeshes(CachedFont *font){
	ttf_glyph_t *glyph = font->ttf->glyphs+ttf_find_glyph(font->ttf,'?');
	font->height = glyph->ybounds[1]-glyph->ybounds[0];
	ListInit(&font->colorVerts,sizeof(ColorVertex),0);
	for (int i = 33; i < COUNT(font->glyphMeshes); i++){
		int index = ttf_find_glyph(font->ttf,i);
		if (index >= 0){
			glyph = font->ttf->glyphs+index;
			ttf_mesh_t *mesh;
			if (ttf_glyph2mesh(glyph,&mesh,TTF_QUALITY_NORMAL,TTF_FEATURES_DFLT) != TTF_DONE){
				font->glyphMeshes[i].index = 0;
				font->glyphMeshes[i].vertexCount = 0;
				font->glyphMeshes[i].width = 0;
				font->glyphMeshes[i].advance = 0;
				continue;
			}
			font->glyphMeshes[i].index = font->colorVerts->used;
			font->glyphMeshes[i].vertexCount = mesh->nfaces*3;
			font->glyphMeshes[i].width = glyph->xbounds[1]-glyph->xbounds[0];
			font->glyphMeshes[i].advance = glyph->advance;
			for (int f = 0; f < mesh->nfaces; f++){
				ColorVertex v[3] = {
					{{mesh->vert[mesh->faces[f].v1].x,mesh->vert[mesh->faces[f].v1].y,0},0},
					{{mesh->vert[mesh->faces[f].v2].x,mesh->vert[mesh->faces[f].v2].y,0},0},
					{{mesh->vert[mesh->faces[f].v3].x,mesh->vert[mesh->faces[f].v3].y,0},0}
				};
				ListAppend(&font->colorVerts,v,COUNT(v));
			}
			ttf_free_mesh(mesh);
		}
	}
}
void LoadFontA(CachedFont *font, char *path){
	ByteArray ba = ByteArrayFromFileA(path);
	ttf_load_from_mem(ba.bytes,ba.size,&font->ttf,false);
	free(ba.bytes);
	if (!font->ttf) FatalErrorA("Failed to load %s",path);
	GenFontMeshes(font);
}
void LoadFontW(CachedFont *font, WCHAR *path){
	ByteArray ba = ByteArrayFromFileW(path);
	ttf_load_from_mem(ba.bytes,ba.size,&font->ttf,false);
	free(ba.bytes);
	if (!font->ttf) FatalErrorA("Failed to load %s",path);
	GenFontMeshes(font);
}
float StringWidth(CachedFont *font, float size, WCHAR *str, int numChars){
	float x = 0;
	for (int i = 0; i < numChars; i++){
		if (str[i]==' ' || str[i]=='\t'){
			if (i == numChars-1) break;
			float advance = font->glyphMeshes['t'].advance*size;
			x += str[i]=='\t' ? 4*advance : advance;
		}
		else {
			GlyphMesh *m = font->glyphMeshes+str[i];
			if (!m->vertexCount) m = font->glyphMeshes+'?';
			x += i == numChars-1 ? m->width*size : m->advance*size;
		}
	}
	return x;
}
void AppendStringMesh(ListHeader **colorVerts, CachedFont *font, int x, int y, int z, float size, uint32_t color, WCHAR *str, int numChars){
	for (int i = 0; i < numChars; i++){
		if (str[i]==' ' || str[i]=='\t'){
			if (i == numChars-1) break;
			float advance = font->glyphMeshes['t'].advance*size;
			x += str[i]=='\t' ? 4*advance : advance;
		}
		else {
			GlyphMesh *m = font->glyphMeshes+str[i];
			if (!m->vertexCount) m = font->glyphMeshes+'?';
			ColorVertex *gv = ListGet(font->colorVerts,m->index);
			for (int j = 0; j < m->vertexCount; j++){
				ColorVertex v;
				v.Position.x = gv[j].Position.x*size + x;
				v.Position.y = gv[j].Position.y*size + y;
				v.Position.z = z;
				v.color = color;
				ListAppend(colorVerts,&v,1);
			}
			x += i == numChars-1 ? m->width*size : m->advance*size;
		}
	}
}
void AppendFormatStringMesh(ListHeader **colorVerts, CachedFont *font, int x, int y, int z, float size, uint32_t color, WCHAR *format, ...){
	va_list args;
	va_start(args,format);
	static WCHAR str[512];
	int len = vswprintf(str,COUNT(str),format,args);
	AppendStringMesh(colorVerts,font,x,y,z,size,color,str,len);
	va_end(args);
}
void AppendFormatCenteredStringMesh(ListHeader **colorVerts, CachedFont *font, int x, int y, int z, float size, uint32_t color, WCHAR *format, ...){
	va_list args;
	va_start(args,format);
	static WCHAR str[512];
	int len = vswprintf(str,COUNT(str),format,args);
	AppendStringMesh(colorVerts,font,x-StringWidth(font,size,str,len)/2+2,y-font->height*size/2,z,size,color,str,len);
	va_end(args);
}
struct UniformColorGouraudDirlightShader UniformColorGouraudDirlightShader = {
	"#version 110\n"
	"attribute vec3 aPosition;\n"
	"attribute vec3 aNormal;\n"
	"uniform vec3 lightDir;\n"
	"uniform mat4 proj;\n"
	"uniform mat4 mtw;\n"
	"uniform vec4 color;\n"
	"varying vec4 Color;\n"
	"void main(){\n"
	"	gl_Position = proj * vec4(aPosition,1.0);\n"
	"	Color = vec4((max(0.0,dot(-lightDir,(mtw*vec4(aNormal,0.0)).xyz))+0.2)*color.rgb,color.a);\n"
	"}",

	"#version 110\n"
	"varying vec4 Color;\n"
	"void main(){\n"
	"	gl_FragColor = Color;\n"
	"}"
};
static void CompileUniformColorGouraudDirlightShader(){
	UniformColorGouraudDirlightShader.id = CompileShader("UniformColorGouraudDirlightShader",UniformColorGouraudDirlightShader.vertSrc,UniformColorGouraudDirlightShader.fragSrc);
	UniformColorGouraudDirlightShader.aPosition = glGetAttribLocation(UniformColorGouraudDirlightShader.id,"aPosition");
	UniformColorGouraudDirlightShader.aNormal = glGetAttribLocation(UniformColorGouraudDirlightShader.id,"aNormal");
	UniformColorGouraudDirlightShader.lightDir = glGetUniformLocation(UniformColorGouraudDirlightShader.id,"lightDir");
	UniformColorGouraudDirlightShader.proj = glGetUniformLocation(UniformColorGouraudDirlightShader.id,"proj");
	UniformColorGouraudDirlightShader.mtw = glGetUniformLocation(UniformColorGouraudDirlightShader.id,"mtw");
	UniformColorGouraudDirlightShader.color = glGetUniformLocation(UniformColorGouraudDirlightShader.id,"color");
}
void UniformColorGouraudDirlightShaderPrepBuffer(){
	glEnableVertexAttribArray(UniformColorGouraudDirlightShader.aPosition);
	glEnableVertexAttribArray(UniformColorGouraudDirlightShader.aNormal);
	glVertexAttribPointer(UniformColorGouraudDirlightShader.aPosition,3,GL_FLOAT,GL_FALSE,sizeof(NormalVertex),0);
	glVertexAttribPointer(UniformColorGouraudDirlightShader.aNormal,3,GL_FLOAT,GL_FALSE,sizeof(NormalVertex),offsetof(NormalVertex,Normal));
}
struct ColorGouraudDirlightShader ColorGouraudDirlightShader = {
	"#version 110\n"
	"attribute vec3 aPosition;\n"
	"attribute vec3 aNormal;\n"
	"attribute vec4 aColor;\n"
	"uniform vec3 lightDir;\n"
	"uniform mat4 proj;\n"
	"uniform mat4 mtw;\n"
	"varying vec4 Color;\n"
	"void main(){\n"
	"	gl_Position = proj * vec4(aPosition,1.0);\n"
	"	Color = vec4((max(0.0,dot(-lightDir,(mtw*vec4(aNormal,0.0)).xyz))+0.2)*aColor.rgb,aColor.a);\n"
	"}",

	"#version 110\n"
	"varying vec4 Color;\n"
	"void main(){\n"
	"	gl_FragColor = Color;\n"
	"}"
};
static void CompileColorGouraudDirlightShader(){
	ColorGouraudDirlightShader.id = CompileShader("ColorGouraudDirlightShader",ColorGouraudDirlightShader.vertSrc,ColorGouraudDirlightShader.fragSrc);
	ColorGouraudDirlightShader.aPosition = glGetAttribLocation(ColorGouraudDirlightShader.id,"aPosition");
	ColorGouraudDirlightShader.aNormal = glGetAttribLocation(ColorGouraudDirlightShader.id,"aNormal");
	ColorGouraudDirlightShader.aColor = glGetAttribLocation(ColorGouraudDirlightShader.id,"aColor");
	ColorGouraudDirlightShader.lightDir = glGetUniformLocation(ColorGouraudDirlightShader.id,"lightDir");
	ColorGouraudDirlightShader.proj = glGetUniformLocation(ColorGouraudDirlightShader.id,"proj");
	ColorGouraudDirlightShader.mtw = glGetUniformLocation(ColorGouraudDirlightShader.id,"mtw");
}
void ColorGouraudDirlightShaderPrepBuffer(){
	glEnableVertexAttribArray(ColorGouraudDirlightShader.aPosition);
	glEnableVertexAttribArray(ColorGouraudDirlightShader.aNormal);
	glEnableVertexAttribArray(ColorGouraudDirlightShader.aColor);
	glVertexAttribPointer(ColorGouraudDirlightShader.aPosition,3,GL_FLOAT,GL_FALSE,sizeof(ColorNormalVertex),0);
	glVertexAttribPointer(ColorGouraudDirlightShader.aNormal,3,GL_FLOAT,GL_FALSE,sizeof(ColorNormalVertex),offsetof(ColorNormalVertex,Normal));
	glVertexAttribPointer(ColorGouraudDirlightShader.aColor,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(ColorNormalVertex),offsetof(ColorNormalVertex,color));
}
FVec3 cubeVerts[]={
	0,1,0, 0,0,0, 0,0,1, 0,0,1, 0,1,1, 0,1,0,
	1,1,1, 1,0,1, 1,0,0, 1,0,0, 1,1,0, 1,1,1,

	0,0,0, 1,0,0, 1,0,1, 1,0,1, 0,0,1, 0,0,0,
	1,1,0, 0,1,0, 0,1,1, 0,1,1, 1,1,1, 1,1,0,

	1,1,0, 1,0,0, 0,0,0, 0,0,0, 0,1,0, 1,1,0,
	0,1,1, 0,0,1, 1,0,1, 1,0,1, 1,1,1, 0,1,1,
};
FVec3 cubeNormals[]={
	-1,0,0,
	1,0,0,

	0,-1,0,
	0,1,0,

	0,0,-1,
	0,0,1
};
void AppendColoredCubeFace(ListHeader **colorNormalVerts, int faceIndex, FVec3 translation, uint32_t color){
	for (int i = 0; i < 6; i++){
		ColorNormalVertex v = {fvec3Add(translation,cubeVerts[faceIndex*6+i]),cubeNormals[faceIndex],color};
		ListAppend(colorNormalVerts,&v,1);
	}
}
void AppendCenteredCube(ListHeader **colorNormalVerts, uint32_t color){
	for (int i = 0; i < 6; i++) AppendColoredCubeFace(colorNormalVerts,i,(FVec3){-0.5f,-0.5f,-0.5f},color);
}
/*AppendCylinder:
innerRadius of 0 will give solid ends
nVertices must be >= 3
*/
void AppendCylinder(ListHeader **normalVerts, float innerRadius, float outerRadius, float height, int nVertices){
	float halfHeight = 0.5f*height;
	NormalVertex *topVerts = MallocOrDie(nVertices*sizeof(NormalVertex));
	NormalVertex *bottomVerts = MallocOrDie(nVertices*sizeof(NormalVertex));
	for (int i = 0; i < nVertices; i++){
		float angle = 2.0f*M_PI*(float)i/(float)nVertices;
		topVerts[i].Position = fvec3Rotated((FVec3){outerRadius,halfHeight,0},(FVec3){0,angle,0});
		topVerts[i].Normal = (FVec3){0,1,0};
		bottomVerts[i].Position = (FVec3){topVerts[i].Position.x,-halfHeight,topVerts[i].Position.z};
		bottomVerts[i].Normal = (FVec3){0,-1,0};
	}
	for (int i = 0; i < nVertices; i++){
		NormalVertex v[6] = {
			topVerts[i],bottomVerts[i],i == (nVertices-1) ? bottomVerts[0] : bottomVerts[i+1],
			i == (nVertices-1) ? bottomVerts[0] : bottomVerts[i+1],i == (nVertices-1) ? topVerts[0] : topVerts[i+1],topVerts[i]
		};
		FVec3 normal = normalFromTriangle(v[0].Position,v[1].Position,v[2].Position);
		for (int i = 0; i < COUNT(v); i++) v[i].Normal = normal;
		ListAppend(normalVerts,v,COUNT(v));
	}
	if (innerRadius <= 0.0f){
		NormalVertex topCenter;
		topCenter.Position = (FVec3){0,halfHeight,0};
		topCenter.Normal = (FVec3){0,1,0};
		NormalVertex bottomCenter;
		bottomCenter.Position = (FVec3){0,-halfHeight,0};
		bottomCenter.Normal = (FVec3){0,-1,0};
		for (int i = 0; i < nVertices; i++){
			ListAppend(normalVerts,&topCenter,1);
			ListAppend(normalVerts,topVerts+i,1);
			ListAppend(normalVerts,i == (nVertices-1) ? topVerts : topVerts+i+1,1);

			ListAppend(normalVerts,i == (nVertices-1) ? bottomVerts : bottomVerts+i+1,1);
			ListAppend(normalVerts,bottomVerts+i,1);
			ListAppend(normalVerts,&bottomCenter,1);
		}
	} else {
		NormalVertex *innerTopVerts = MallocOrDie(nVertices*sizeof(NormalVertex));
		NormalVertex *innerBottomVerts = MallocOrDie(nVertices*sizeof(NormalVertex));
		for (int i = 0; i < nVertices; i++){
			innerTopVerts[i].Position = topVerts[i].Position;
			innerTopVerts[i].Position.x *= innerRadius/outerRadius;
			innerTopVerts[i].Position.z *= innerRadius/outerRadius;
			innerTopVerts[i].Normal = topVerts[i].Normal;
			innerBottomVerts[i].Position = (FVec3){innerTopVerts[i].Position.x,-halfHeight,innerTopVerts[i].Position.z};
			innerBottomVerts[i].Normal = bottomVerts[i].Normal;
		}
		for (int i = 0; i < nVertices; i++){
			ListAppend(normalVerts,i == (nVertices-1) ? innerTopVerts : innerTopVerts+i+1,1);
			ListAppend(normalVerts,innerTopVerts+i,1);
			ListAppend(normalVerts,topVerts+i,1);

			ListAppend(normalVerts,topVerts+i,1);
			ListAppend(normalVerts,i == (nVertices-1) ? topVerts : topVerts+i+1,1);
			ListAppend(normalVerts,i == (nVertices-1) ? innerTopVerts : innerTopVerts+i+1,1);

			ListAppend(normalVerts,bottomVerts+i,1);
			ListAppend(normalVerts,innerBottomVerts+i,1);
			ListAppend(normalVerts,i == (nVertices-1) ? innerBottomVerts : innerBottomVerts+i+1,1);

			ListAppend(normalVerts,i == (nVertices-1) ? innerBottomVerts : innerBottomVerts+i+1,1);
			ListAppend(normalVerts,i == (nVertices-1) ? bottomVerts : bottomVerts+i+1,1);
			ListAppend(normalVerts,bottomVerts+i,1);

			NormalVertex v[6] = {
				i == (nVertices-1) ? innerBottomVerts[0] : innerBottomVerts[i+1],innerBottomVerts[i],innerTopVerts[i],
				innerTopVerts[i],i == (nVertices-1) ? innerTopVerts[0] : innerTopVerts[i+1],i == (nVertices-1) ? innerBottomVerts[0] : innerBottomVerts[i+1]
			};
			FVec3 normal = normalFromTriangle(v[0].Position,v[1].Position,v[2].Position);
			for (int i = 0; i < COUNT(v); i++) v[i].Normal = normal;
			ListAppend(normalVerts,v,COUNT(v));
		}
		free(innerTopVerts);
		free(innerBottomVerts);
	}
	free(topVerts);
	free(bottomVerts);
}
void AppendColoredCylinder(ListHeader **colorNormalVerts, float innerRadius, float outerRadius, float height, int nVertices, uint32_t color){
	float halfHeight = 0.5f*height;
	ColorNormalVertex *topVerts = MallocOrDie(nVertices*sizeof(ColorNormalVertex));
	ColorNormalVertex *bottomVerts = MallocOrDie(nVertices*sizeof(ColorNormalVertex));
	for (int i = 0; i < nVertices; i++){
		float angle = 2.0f*M_PI*(float)i/(float)nVertices;
		topVerts[i].Position = fvec3Rotated((FVec3){outerRadius,halfHeight,0},(FVec3){0,angle,0});
		topVerts[i].Normal = (FVec3){0,1,0};
		topVerts[i].color = color;
		bottomVerts[i].Position = (FVec3){topVerts[i].Position.x,-halfHeight,topVerts[i].Position.z};
		bottomVerts[i].Normal = (FVec3){0,-1,0};
		bottomVerts[i].color = color;
	}
	for (int i = 0; i < nVertices; i++){
		ColorNormalVertex v[6] = {
			topVerts[i],bottomVerts[i],i == (nVertices-1) ? bottomVerts[0] : bottomVerts[i+1],
			i == (nVertices-1) ? bottomVerts[0] : bottomVerts[i+1],i == (nVertices-1) ? topVerts[0] : topVerts[i+1],topVerts[i]
		};
		FVec3 normal = normalFromTriangle(v[0].Position,v[1].Position,v[2].Position);
		for (int i = 0; i < COUNT(v); i++) v[i].Normal = normal;
		ListAppend(colorNormalVerts,v,COUNT(v));
	}
	if (innerRadius <= 0.0f){
		ColorNormalVertex topCenter;
		topCenter.Position = (FVec3){0,halfHeight,0};
		topCenter.Normal = (FVec3){0,1,0};
		topCenter.color = color;
		ColorNormalVertex bottomCenter;
		bottomCenter.Position = (FVec3){0,-halfHeight,0};
		bottomCenter.Normal = (FVec3){0,-1,0};
		bottomCenter.color = color;
		for (int i = 0; i < nVertices; i++){
			ListAppend(colorNormalVerts,&topCenter,1);
			ListAppend(colorNormalVerts,topVerts+i,1);
			ListAppend(colorNormalVerts,i == (nVertices-1) ? topVerts : topVerts+i+1,1);

			ListAppend(colorNormalVerts,i == (nVertices-1) ? bottomVerts : bottomVerts+i+1,1);
			ListAppend(colorNormalVerts,bottomVerts+i,1);
			ListAppend(colorNormalVerts,&bottomCenter,1);
		}
	} else {
		ColorNormalVertex *innerTopVerts = MallocOrDie(nVertices*sizeof(ColorNormalVertex));
		ColorNormalVertex *innerBottomVerts = MallocOrDie(nVertices*sizeof(ColorNormalVertex));
		for (int i = 0; i < nVertices; i++){
			innerTopVerts[i].Position = topVerts[i].Position;
			innerTopVerts[i].Position.x *= innerRadius/outerRadius;
			innerTopVerts[i].Position.z *= innerRadius/outerRadius;
			innerTopVerts[i].Normal = topVerts[i].Normal;
			innerTopVerts[i].color = color;
			innerBottomVerts[i].Position = (FVec3){innerTopVerts[i].Position.x,-halfHeight,innerTopVerts[i].Position.z};
			innerBottomVerts[i].Normal = bottomVerts[i].Normal;
			innerBottomVerts[i].color = color;
		}
		for (int i = 0; i < nVertices; i++){
			ListAppend(colorNormalVerts,i == (nVertices-1) ? innerTopVerts : innerTopVerts+i+1,1);
			ListAppend(colorNormalVerts,innerTopVerts+i,1);
			ListAppend(colorNormalVerts,topVerts+i,1);

			ListAppend(colorNormalVerts,topVerts+i,1);
			ListAppend(colorNormalVerts,i == (nVertices-1) ? topVerts : topVerts+i+1,1);
			ListAppend(colorNormalVerts,i == (nVertices-1) ? innerTopVerts : innerTopVerts+i+1,1);

			ListAppend(colorNormalVerts,bottomVerts+i,1);
			ListAppend(colorNormalVerts,innerBottomVerts+i,1);
			ListAppend(colorNormalVerts,i == (nVertices-1) ? innerBottomVerts : innerBottomVerts+i+1,1);

			ListAppend(colorNormalVerts,i == (nVertices-1) ? innerBottomVerts : innerBottomVerts+i+1,1);
			ListAppend(colorNormalVerts,i == (nVertices-1) ? bottomVerts : bottomVerts+i+1,1);
			ListAppend(colorNormalVerts,bottomVerts+i,1);

			ColorNormalVertex v[6] = {
				i == (nVertices-1) ? innerBottomVerts[0] : innerBottomVerts[i+1],innerBottomVerts[i],innerTopVerts[i],
				innerTopVerts[i],i == (nVertices-1) ? innerTopVerts[0] : innerTopVerts[i+1],i == (nVertices-1) ? innerBottomVerts[0] : innerBottomVerts[i+1]
			};
			FVec3 normal = normalFromTriangle(v[0].Position,v[1].Position,v[2].Position);
			for (int i = 0; i < COUNT(v); i++) v[i].Normal = normal;
			ListAppend(colorNormalVerts,v,COUNT(v));
		}
		free(innerTopVerts);
		free(innerBottomVerts);
	}
	free(topVerts);
	free(bottomVerts);
}
struct RoundedRectangleShader RoundedRectangleShader = {
	"#version 110\n"
	"attribute vec3 aPosition;\n"
	"attribute vec4 aRectangle;\n"//xy: center, zw: half extents
	"attribute float aRoundingRadius;\n"
	"attribute vec4 aColor;\n"
	"attribute vec4 aIconColor;\n"
	"uniform mat4 proj;\n"
	"varying vec4 Rectangle;\n"
	"varying float RoundingRadius;\n"
	"varying vec4 Color;\n"
	"varying vec4 IconColor;\n"
	"void main(){\n"
	"	gl_Position = proj * vec4(aPosition,1.0);\n"
	"	Rectangle = aRectangle;\n"
	"	RoundingRadius = aRoundingRadius;\n"
	"	Color = aColor;\n"
	"	IconColor = aIconColor;\n"
	"}",

	"#version 110\n"
	"#define PI 3.14159265\n"
	"varying vec4 Rectangle;\n"
	"varying float RoundingRadius;\n"
	"varying vec4 Color;\n"//alpha selects shading type
	"varying vec4 IconColor;\n"//alpha selects icon type
	"float DistanceAABB(vec2 p, vec2 he, float r){\n"//based on qmopey's shader: https://www.shadertoy.com/view/cts3W2
	"	vec2 d = abs(p) - he + r;\n"
	"	return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;\n"
	"}\n"
	// The MIT License
	// sdEquilateralTriangle Copyright © 2017 Inigo Quilez https://www.shadertoy.com/view/Xl2yDW
	// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	"float DistanceEquilateralTriangle(vec2 p, float r){\n"
	"	float k = sqrt(3.0);\n"
	"	p.x = abs(p.x) - r;\n"
	"	p.y = p.y + r/k;\n"
	"	if( p.x+k*p.y>0.0 ) p=vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;\n"
	"	p.x -= clamp( p.x, -2.0*r, 0.0 );\n"
	"	if (p.y > 0.0) return -length(p);\n"
	"	else return length(p);\n"
	"}\n"
	"vec2 rotate(vec2 p, float angle){\n"
	"	float c = cos(angle);\n"
	"	float s = sin(angle);\n"
	"	mat2 m = mat2(c,-s,s,c);\n"
	"	return m * p;\n"
	"}\n"
	"void main(){\n"
	"	vec2 p = gl_FragCoord.xy-Rectangle.xy;\n"
	"	vec2 pn = p;\n"
	"	if (pn.x < 0.0) pn.x += min(abs(pn.x),Rectangle.z-RoundingRadius);\n"
	"	else pn.x -= min(pn.x,Rectangle.z-RoundingRadius);\n"
	"	float d = DistanceAABB(p,Rectangle.zw,RoundingRadius);\n"
	"	if (Color.a < 0.25){\n"//CHANNEL
	"		vec3 outerColor = Color.rgb+dot(pn/RoundingRadius,vec2(1,-1))*vec3(0.1);\n"
	"		gl_FragColor = vec4(outerColor,1.0-clamp(d,0.0,1.0));\n"
	"		return;\n"
	"	}\n"
	"	vec3 outerColor = Color.rgb+dot(pn/RoundingRadius,vec2(-1,1))*vec3(0.1);\n"
	"	float innerRadius = RoundingRadius*0.8;\n"
	"	vec3 innerColorShift;\n"
	"	vec3 innerColor;\n"
	"	if (Color.a < 0.5){\n"//FLAT
	"		innerColorShift = vec3(0.0);\n"
	"		innerColor = Color.rgb;\n"
	"	} else if (Color.a < 0.75){\n"//DISH
	"		innerColorShift = dot(pn/innerRadius,vec2(1,-1))*vec3(0.1);\n"
	"		innerColor = Color.rgb+innerColorShift;\n"
	"	} else {\n"//CONE
	"		innerColorShift = dot(normalize(pn/innerRadius),vec2(1,-1))*vec3(0.1);\n"
	"		innerColor = Color.rgb+innerColorShift;\n"
	"	}\n"
	"	if (IconColor.a < 0.1666) gl_FragColor = vec4(mix(innerColor,outerColor,clamp(d+(RoundingRadius-innerRadius),0.0,1.0)),1.0-clamp(d,0.0,1.0));\n"
	"	else if (IconColor.a < 0.333){\n"
	"		vec3 iconColorShaded = IconColor.rgb+innerColorShift;\n"
	"		float iconD = DistanceEquilateralTriangle(rotate(p,-PI/2.0),RoundingRadius/4.0)-(RoundingRadius/16.0);\n"
	"		gl_FragColor = vec4(mix(mix(iconColorShaded,innerColor,clamp(iconD,0.0,1.0)),outerColor,clamp(d+(RoundingRadius-innerRadius),0.0,1.0)),1.0-clamp(d,0.0,1.0));\n"
	"	} else if (IconColor.a < 0.5){\n"
	"		vec3 iconColorShaded = IconColor.rgb+innerColorShift;\n"
	"		float iconD = DistanceEquilateralTriangle(rotate(p,PI/2.0),RoundingRadius/4.0)-(RoundingRadius/16.0);\n"
	"		gl_FragColor = vec4(mix(mix(iconColorShaded,innerColor,clamp(iconD,0.0,1.0)),outerColor,clamp(d+(RoundingRadius-innerRadius),0.0,1.0)),1.0-clamp(d,0.0,1.0));\n"
	"	} else if (IconColor.a < 0.666){\n"
	"		vec3 iconColorShaded = IconColor.rgb+innerColorShift;\n"
	"		vec2 halfExtents = vec2(innerRadius/9.0,innerRadius/3.0);\n"
	"		vec2 spacing = vec2(halfExtents.x*2.0,0.0);\n"
	"		float iconD = min(DistanceAABB(p+spacing,halfExtents,halfExtents.x),DistanceAABB(p-spacing,halfExtents,halfExtents.x));\n"
	"		gl_FragColor = vec4(mix(mix(iconColorShaded,innerColor,clamp(iconD,0.0,1.0)),outerColor,clamp(d+(RoundingRadius-innerRadius),0.0,1.0)),1.0-clamp(d,0.0,1.0));\n"
	"	} else if (IconColor.a < 0.833){\n"
	"		vec3 iconColorShaded = IconColor.rgb+innerColorShift;\n"
	"		vec2 halfExtents = vec2(RoundingRadius/16.0,RoundingRadius/4.0);\n"
	"		float playRadius = RoundingRadius/5.0;\n"
	"		float playRoundingRadius = playRadius/4.0;\n"
	"		float iconD = min(DistanceEquilateralTriangle(rotate(p+vec2(halfExtents.x,0.0),-PI/2.0),playRadius)-playRoundingRadius,DistanceAABB(p-vec2(playRadius+playRoundingRadius-halfExtents.x,0.0),halfExtents,halfExtents.x));\n"
	"		gl_FragColor = vec4(mix(mix(iconColorShaded,innerColor,clamp(iconD,0.0,1.0)),outerColor,clamp(d+(RoundingRadius-innerRadius),0.0,1.0)),1.0-clamp(d,0.0,1.0));\n"
	"	} else {\n"
	"		vec3 iconColorShaded = IconColor.rgb+innerColorShift;\n"
	"		vec2 halfExtents = vec2(RoundingRadius/16.0,RoundingRadius/4.0);\n"
	"		float playRadius = RoundingRadius/5.0;\n"
	"		float playRoundingRadius = playRadius/4.0;\n"
	"		float iconD = min(DistanceEquilateralTriangle(rotate(p-vec2(halfExtents.x,0.0),PI/2.0),playRadius)-playRoundingRadius,DistanceAABB(p+vec2(playRadius+playRoundingRadius-halfExtents.x,0.0),halfExtents,halfExtents.x));\n"
	"		gl_FragColor = vec4(mix(mix(iconColorShaded,innerColor,clamp(iconD,0.0,1.0)),outerColor,clamp(d+(RoundingRadius-innerRadius),0.0,1.0)),1.0-clamp(d,0.0,1.0));\n"
	"	}\n"
	"}\n"
};
static void CompileRoundedRectangleShader(){
	RoundedRectangleShader.id = CompileShader("RoundedRectangleShader",RoundedRectangleShader.vertSrc,RoundedRectangleShader.fragSrc);
	RoundedRectangleShader.aPosition = glGetAttribLocation(RoundedRectangleShader.id,"aPosition");
	RoundedRectangleShader.aRectangle = glGetAttribLocation(RoundedRectangleShader.id,"aRectangle");
	RoundedRectangleShader.aRoundingRadius = glGetAttribLocation(RoundedRectangleShader.id,"aRoundingRadius");
	RoundedRectangleShader.aColor = glGetAttribLocation(RoundedRectangleShader.id,"aColor");
	RoundedRectangleShader.aIconColor = glGetAttribLocation(RoundedRectangleShader.id,"aIconColor");
	RoundedRectangleShader.proj = glGetUniformLocation(RoundedRectangleShader.id,"proj");
}
void RoundedRectangleShaderPrepBuffer(){
	glEnableVertexAttribArray(RoundedRectangleShader.aPosition);
	glEnableVertexAttribArray(RoundedRectangleShader.aRectangle);
	glEnableVertexAttribArray(RoundedRectangleShader.aRoundingRadius);
	glEnableVertexAttribArray(RoundedRectangleShader.aColor);
	glEnableVertexAttribArray(RoundedRectangleShader.aIconColor);
	glVertexAttribPointer(RoundedRectangleShader.aPosition,3,GL_FLOAT,GL_FALSE,sizeof(RoundedRectangleVertex),0);
	glVertexAttribPointer(RoundedRectangleShader.aRectangle,4,GL_FLOAT,GL_FALSE,sizeof(RoundedRectangleVertex),offsetof(RoundedRectangleVertex,Rectangle));
	glVertexAttribPointer(RoundedRectangleShader.aRoundingRadius,1,GL_FLOAT,GL_FALSE,sizeof(RoundedRectangleVertex),offsetof(RoundedRectangleVertex,RoundingRadius));
	glVertexAttribPointer(RoundedRectangleShader.aColor,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(RoundedRectangleVertex),offsetof(RoundedRectangleVertex,color));
	glVertexAttribPointer(RoundedRectangleShader.aIconColor,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(RoundedRectangleVertex),offsetof(RoundedRectangleVertex,IconColor));
}
void AppendRoundedRectangle(ListHeader **verts, int x, int y, int z, int halfWidth, int halfHeight, float RoundingRadius, uint32_t color, uint32_t IconColor){
	//we have to rotate the bounding rectangle on the CPU, because otherwise we'd have to change the ortho matrix and do a separate draw call for every RoundedRect.
	RoundedRectangleVertex v[6];
	int hwp = halfWidth+4;
	int hhp = halfHeight+4;
	v[0].Position = (FVec3){-hwp,hhp,z};
	v[1].Position = (FVec3){-hwp,-hhp,z};
	v[2].Position = (FVec3){hwp,-hhp,z};
	v[3].Position = v[2].Position;
	v[4].Position = (FVec3){hwp,hhp,z};
	v[5].Position = v[0].Position;
	for (int i = 0; i < COUNT(v); i++){
		v[i].Position = fvec3Add(v[i].Position,(FVec3){x,y,0});
		v[i].Rectangle = (FVec4){x,y,halfWidth,halfHeight};
		v[i].RoundingRadius = RoundingRadius;
		v[i].color = color;
		v[i].IconColor = IconColor;
	}
	ListAppend(verts,v,COUNT(v));
}
struct TextureShader TextureShader = {
	"#version 110\n"
	"attribute vec3 aPosition;\n"
	"attribute vec2 aTexcoord;\n"
	"uniform mat4 uProj;\n"
	"varying vec2 Texcoord;\n"
	"void main(){\n"
	"	gl_Position = uProj * vec4(aPosition,1.0f);\n"
	"	Texcoord = aTexcoord;\n"
	"}",

	"#version 110\n"
	"uniform sampler2D uTex;\n"
	"varying vec2 Texcoord;\n"
	"void main(){\n"
	"	gl_FragColor = texture2D(uTex,Texcoord);\n"
	"}"
};
static void CompileTextureShader(){
	TextureShader.id = CompileShader("TextureShader",TextureShader.vertSrc,TextureShader.fragSrc);
	TextureShader.aPosition = glGetAttribLocation(TextureShader.id,"aPosition");
	TextureShader.aTexcoord = glGetAttribLocation(TextureShader.id,"aTexcoord");
	TextureShader.uProj = glGetUniformLocation(TextureShader.id,"uProj");
	TextureShader.uTex = glGetUniformLocation(TextureShader.id,"uTex");
}
void TextureShaderPrepBuffer(){
	glEnableVertexAttribArray(TextureShader.aPosition);
	glEnableVertexAttribArray(TextureShader.aTexcoord);
	glVertexAttribPointer(TextureShader.aPosition,3,GL_FLOAT,GL_FALSE,sizeof(TextureVertex),0);
	glVertexAttribPointer(TextureShader.aTexcoord,2,GL_FLOAT,GL_FALSE,sizeof(TextureVertex),offsetof(TextureVertex,Texcoord));
}
static void CompileShaders(){
	CompileColorShader();
	CompileUniformColorGouraudDirlightShader();
	CompileColorGouraudDirlightShader();
	CompileRoundedRectangleShader();
	CompileTextureShader();
}

#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
static void FatalError(const char* message){
	MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
	ExitProcess(0);
}
static int StringsAreEqual(const char* src, const char* dst, size_t dstlen){
	while (*src && dstlen-- && *dst)
	{
		if (*src++ != *dst++)
		{
			return 0;
		}
	}
	return (dstlen && *src == *dst) || (!dstlen && *src == 0);
}

HWND DarkGLMakeWindow(int iconId, WCHAR *title, int clientWidth, int clientHeight, WNDPROC windowProc){
	/*
	Modified from https://gist.github.com/mmozeiko/ed2ad27f75edf9c26053ce332a1f6647
	*/
	if (!wglChoosePixelFormatARB){
		// to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx
		HWND dummy = CreateWindowExW(
			0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL);
		Assert(dummy && "Failed to create dummy window");

		HDC dc = GetDC(dummy);
		Assert(dc && "Failed to get device context for dummy window");

		PIXELFORMATDESCRIPTOR desc =
		{
			.nSize = sizeof(desc),
			.nVersion = 1,
			.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			.iPixelType = PFD_TYPE_RGBA,
			.cColorBits = 24,
		};

		int format = ChoosePixelFormat(dc, &desc);
		if (!format)
		{
			FatalError("Cannot choose OpenGL pixel format for dummy window!");
		}

		int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
		Assert(ok && "Failed to describe OpenGL pixel format");

		// reason to create dummy window is that SetPixelFormat can be called only once for the window
		if (!SetPixelFormat(dc, format, &desc))
		{
			FatalError("Cannot set OpenGL pixel format for dummy window!");
		}

		HGLRC rc = wglCreateContext(dc);
		Assert(rc && "Failed to create OpenGL context for dummy window");

		ok = wglMakeCurrent(dc, rc);
		Assert(ok && "Failed to make current OpenGL context for dummy window");

		// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
		PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
			(void*)wglGetProcAddress("wglGetExtensionsStringARB");
		if (!wglGetExtensionsStringARB)
		{
			FatalError("OpenGL does not support WGL_ARB_extensions_string extension!");
		}

		const char* ext = wglGetExtensionsStringARB(dc);
		Assert(ext && "Failed to get OpenGL WGL extension string");

		const char* start = ext;
		for (;;)
		{
			while (*ext != 0 && *ext != ' ')
			{
				ext++;
			}

			size_t length = ext - start;
			if (StringsAreEqual("WGL_ARB_pixel_format", start, length))
			{
				// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
				wglChoosePixelFormatARB = (void*)wglGetProcAddress("wglChoosePixelFormatARB");
			}
			else if (StringsAreEqual("WGL_ARB_create_context", start, length))
			{
				// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
				wglCreateContextAttribsARB = (void*)wglGetProcAddress("wglCreateContextAttribsARB");
			}
			else if (StringsAreEqual("WGL_EXT_swap_control", start, length))
			{
				// https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
				wglSwapIntervalEXT = (void*)wglGetProcAddress("wglSwapIntervalEXT");
			}

			if (*ext == 0)
			{
				break;
			}

			ext++;
			start = ext;
		}

		if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT)
		{
			FatalError("OpenGL does not support required WGL extensions for modern context!");
		}

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(rc);
		ReleaseDC(dummy, dc);
		DestroyWindow(dummy);
	}
	// register window class to have custom WindowProc callback
	WNDCLASSEXW wc =
	{
		.style = CS_HREDRAW | CS_VREDRAW,
		.cbSize = sizeof(wc),
		.lpfnWndProc = windowProc,
		.hInstance = GetModuleHandleW(0),
		.hIcon = LoadIconW(GetModuleHandleW(0),MAKEINTRESOURCEW(iconId)),
		.lpszClassName = L"opengl_window_class",
	};
	ATOM atom = RegisterClassExW(&wc);
	Assert(atom && "Failed to register window class");

	RECT wr = {0,0,clientWidth,clientHeight};
	AdjustWindowRect(&wr,WS_OVERLAPPEDWINDOW,FALSE);
	int wndWidth = wr.right-wr.left;
	int wndHeight = wr.bottom-wr.top;
	HWND hwnd = CreateWindowExW(WS_EX_APPWINDOW,wc.lpszClassName,title,WS_OVERLAPPEDWINDOW,GetSystemMetrics(SM_CXSCREEN)/2-wndWidth/2,GetSystemMetrics(SM_CYSCREEN)/2-wndHeight/2,wndWidth,wndHeight,0,0,wc.hInstance,0);
	Assert(hwnd && "Failed to create window");

	HDC dc = GetDC(hwnd);
	Assert(dc && "Failed to window device context");

	// set pixel format for OpenGL context
	{
		int attrib[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
			WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB,     24,
			WGL_DEPTH_BITS_ARB,     24,
			WGL_STENCIL_BITS_ARB,   8,

			// uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
			//WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

			// uncomment for multisampled framebuffer, from WGL_ARB_multisample extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
			WGL_SAMPLE_BUFFERS_ARB, 1,
			WGL_SAMPLES_ARB,        4, // 4x MSAA

			0,
		};

		int format;
		UINT formats;
		if (!wglChoosePixelFormatARB(dc, attrib, NULL, 1, &format, &formats) || formats == 0)
		{
			FatalError("OpenGL does not support required pixel format!");
		}

		PIXELFORMATDESCRIPTOR desc = { .nSize = sizeof(desc) };
		int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
		Assert(ok && "Failed to describe OpenGL pixel format");

		if (!SetPixelFormat(dc, format, &desc))
		{
			FatalError("Cannot set OpenGL selected pixel format!");
		}
	}

	// create modern OpenGL context
	{
		int attrib[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
			WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			0,
		};

		HGLRC rc = wglCreateContextAttribsARB(dc, NULL, attrib);
		if (!rc)
		{
			FatalError("Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");
		}

		BOOL ok = wglMakeCurrent(dc, rc);
		Assert(ok && "Failed to make current OpenGL context");

		// load OpenGL functions
#define X(type, name) name = (type)wglGetProcAddress(#name); Assert(name);
		GL_FUNCTIONS(X)
#undef X
	}

	wglSwapIntervalEXT(1);

	DWORD darkTitlebar = 1;
	int DwmwaUseImmersiveDarkMode = 20,
		DwmwaUseImmersiveDarkModeBefore20h1 = 19;
	SUCCEEDED(DwmSetWindowAttribute(hwnd, DwmwaUseImmersiveDarkMode, &darkTitlebar, sizeof(darkTitlebar))) ||
		SUCCEEDED(DwmSetWindowAttribute(hwnd, DwmwaUseImmersiveDarkModeBefore20h1, &darkTitlebar, sizeof(darkTitlebar)));

	CompileShaders();

	srand(time(0));

	return hwnd;
}