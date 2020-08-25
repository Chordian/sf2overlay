#pragma once

#include <windows.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>	// Don't upgrade SDL2_image-2.0.4 to 2.0.5 (it doesn't work with newer SDL2 versions)
						// @link https://www.libsdl.org/projects/SDL_image/
class Overlay
{
public:

	Overlay();
	~Overlay();

	bool Init();
	HWND Show();
	void Close();

	HANDLE Run(LPCTSTR inPath, char* inCommandLine);
	bool IsRunning(HANDLE inProcess, int inTimeout = 1000);
	HWND GetWindow(HANDLE inProcess);
	void SetPos(int inX, int inY);

private:

	SDL_Window* m_Window;
	SDL_Surface* m_ScreenSurface;
	SDL_Surface* m_Image;

	STARTUPINFO m_StartupInfo;
	PROCESS_INFORMATION m_ProcessInfo;
};