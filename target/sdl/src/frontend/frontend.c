// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "core/core.h"
#include "frontend/frontend.h"

// =============================================================================
// Constants declaration
// =============================================================================
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define C_RGB_MASK_RED 0xff000000
#define C_RGB_MASK_GREEN 0x00ff0000
#define C_RGB_MASK_BLUE 0x0000ff00
#define C_RGB_MASK_ALPHA 0x000000ff
#define C_RGB_SHIFT_RED 24
#define C_RGB_SHIFT_GREEN 16
#define C_RGB_SHIFT_BLUE 8
#define C_RGB_SHIFT_ALPHA 0
#else
#define C_RGB_MASK_RED 0x000000ff
#define C_RGB_MASK_GREEN 0x0000ff00
#define C_RGB_MASK_BLUE 0x00ff0000
#define C_RGB_MASK_ALPHA 0xff000000
#define C_RGB_SHIFT_RED 0
#define C_RGB_SHIFT_GREEN 8
#define C_RGB_SHIFT_BLUE 16
#define C_RGB_SHIFT_ALPHA 24
#endif

#define C_PW_SCREEN_WIDTH 96
#define C_PW_SCREEN_HEIGHT 64
#define C_PW_SCREEN_SCALE 2

// =============================================================================
// Private variables declarations
// =============================================================================
/**
 * @brief This variable stores the pointer to the SDL_Window object.
 */
static SDL_Window *s_window;

/**
 * @brief This variable stores the pointer to the SDL_Surface object that
 *        contains the framebuffer of the window.
 */
static SDL_Surface *s_windowSurface;

/**
 * @brief This variable stores the pointer to the SDL_Surface object that
 *        stores the video output of the core converted to SDL_Surface format.
 */
static SDL_Surface *s_bufferSurface;

// =============================================================================
// Public functions definitions
// =============================================================================
int frontendInit(void) {
    // Initialize SDL2
    int l_returnValue = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    if(l_returnValue != 0) {
        fprintf(
            stderr,
            "SDL_Init() returned error code %d: %s\n",
            l_returnValue,
            SDL_GetError()
        );

        return 1;
    }

    // Create window
    s_window = SDL_CreateWindow(
        "Pokéwalker",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        C_PW_SCREEN_WIDTH * C_PW_SCREEN_SCALE,
        C_PW_SCREEN_HEIGHT * C_PW_SCREEN_SCALE,
        SDL_WINDOW_SHOWN
    );

    if(s_window == NULL) {
        fprintf(
            stderr,
            "SDL_CreateWindow() returned error: %s\n",
            SDL_GetError()
        );

        return 1;
    }

    // Create buffer surface
    s_bufferSurface = SDL_CreateRGBSurface(
        SDL_SWSURFACE,
        C_PW_SCREEN_WIDTH,
        C_PW_SCREEN_HEIGHT,
        32,
        C_RGB_MASK_RED,
        C_RGB_MASK_GREEN,
        C_RGB_MASK_BLUE,
        C_RGB_MASK_ALPHA
    );

    if(s_bufferSurface == NULL) {
        fprintf(
            stderr,
            "SDL_CreateRGBSurface returned error: %s\n",
            SDL_GetError()
        );

        return 1;
    }

    // Get window surface
    s_windowSurface = SDL_GetWindowSurface(s_window);

    if(s_windowSurface == NULL) {
        fprintf(
            stderr,
            "SDL_GetWindowSurface returned error: %s\n",
            SDL_GetError()
        );

        return 1;
    }

    return 0;
}

void frontendOnVBlank(void) {
    // Get the pointer to the core video buffer
    const uint32_t *l_coreFramebuffer = coreGetVideoBuffer();

    // Copy the core video buffer to the SDL surface
    memcpy(
        s_bufferSurface->pixels,
        l_coreFramebuffer,
        C_PW_SCREEN_WIDTH * C_PW_SCREEN_HEIGHT * 4
    );

    // Blit the buffer to the window
    SDL_BlitScaled(
        s_bufferSurface,
        NULL,
        s_windowSurface,
        NULL
    );

    // Refresh the window
    SDL_UpdateWindowSurface(s_window);

    // Process events
    SDL_Event l_event;

    while(SDL_PollEvent(&l_event) != 0) {
        switch(l_event.type) {
            case SDL_WINDOWEVENT:
                switch(l_event.window.type) {
                    case SDL_WINDOWEVENT_CLOSE:
                        exit(0);
                        break;

                    default:
                        break;
                }

                break;

            default:
                break;
        }
    }
}
