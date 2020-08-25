#include "classes/overlay.h"

#define OVERLAY_Y_OFFSET 107

int main(int argc, char* argv[])
{
    Overlay overlay;
    if (!overlay.Init()) return 0;

    HWND hwnd_overlay = overlay.Show();
    HANDLE sf2 = overlay.Run("..\\SIDFactoryII.exe", *argv);

    SDL_Delay(400); // HWND is 0 if too fast
    HWND hwnd_sf2 = overlay.GetWindow(sf2),
        foreground, last_foreground = GetForegroundWindow();

    RECT rc, rcOverlay, rcSF2;
    SDL_Event event;

    // Don't set milliseconds to 0 (smooth but eats CPU time)
    while (overlay.IsRunning(sf2, 10))
    {
        // Must poll messages or Windows will think the program is dead
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) return 0; // Close overlay only

        GetWindowRect(hwnd_sf2, &rcSF2);
        GetWindowRect(hwnd_overlay, &rcOverlay);
        CopyRect(&rc, &rcSF2);

        OffsetRect(&rcOverlay, -rcOverlay.left, -rcOverlay.top);
        OffsetRect(&rc, -rc.left, -rc.top);
        OffsetRect(&rc, -rcOverlay.right, -rcOverlay.bottom);

        // Move the overlay in sync with SF2
        SetWindowPos(hwnd_overlay,
            HWND_TOP,
            rcSF2.left + (rc.right / 2),
            rcSF2.top + (rc.bottom / 2) - OVERLAY_Y_OFFSET,
            0, 0, // Ignore size
            SWP_NOSIZE);

        foreground = GetForegroundWindow();
        if (foreground != last_foreground)
        {
            last_foreground = foreground;
            if (foreground == hwnd_sf2)
            {
                // If SF2 is made active again then also set the overlay active just behind it
                SetWindowPos(hwnd_overlay, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
                SetWindowPos(hwnd_overlay, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                SetWindowPos(hwnd_sf2, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                SetWindowPos(hwnd_sf2, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            }
            else if (foreground == hwnd_overlay)
                // If the overlay is made active then make sure SF2 gets the attention on top of it
                SetForegroundWindow(hwnd_sf2);
        }
    }

    return 0;
}