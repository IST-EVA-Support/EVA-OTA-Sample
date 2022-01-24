#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
char* getPackageID(void);
#elif _WIN32
__declspec(dllexport) char* getPackageID(void);
#endif

#ifdef __linux__
char* getDevID(void);
#elif _WIN32
__declspec(dllexport) char* getDevID(void);
#endif

#ifdef __linux__
char* getDevBrand(void);
#elif _WIN32
__declspec(dllexport) char* getDevBrand(void);
#endif

#ifdef __linux__
char* getDevModel(void);
#elif _WIN32
__declspec(dllexport) char* getDevModel(void);
#endif

#ifdef __linux__
int setProjToken(char* token);
#elif _WIN32
__declspec(dllexport) int setProjToken(char* token);
#endif

#ifdef __linux__
char* getProjToken(void);
#elif _WIN32
__declspec(dllexport) char* getProjToken(void);
#endif


#ifdef __cplusplus
}
#endif

#endif
