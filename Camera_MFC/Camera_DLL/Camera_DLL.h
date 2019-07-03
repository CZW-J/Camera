

#pragma once


#ifndef __CAMERA_DLL_H_
#define __CAMERA_DLL_H_


#ifndef  WINAPI
#define WINAPI _stdcall
#endif

#define CAMERA_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	CAMERA_API char* WINAPI getCamera();

	CAMERA_API char* WINAPI openCamera(HWND winHwnd, int resolutionX, int resolutionY, int device);

	CAMERA_API char* WINAPI closeCamera();

#ifdef __cplusplus
      }
#endif

#endif