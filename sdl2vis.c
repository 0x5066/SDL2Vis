/*

compile with
g++ c64.c -lSDL2

*/

#include <stdio.h>
#include "SDL2/SDL.h"
#include <vector>
#include <algorithm>
#include <iostream>

#define WIDTH 75
#define HEIGHT 16
#define ZOOM 12

int SHIFT_AMOUNT = 0;
int DIRECTION = 1;

typedef struct {
    Uint8 r, g, b, a;
} Color;

Color colors[] = {
    {0, 0, 0, 255},        // color 0 = black
    {24, 33, 41, 255},     // color 1 = grey for dots
    {239, 49, 16, 255},    // color 2 = top of spec
    {206, 41, 16, 255},    // 3
    {214, 90, 0, 255},     // 4
    {214, 102, 0, 255},    // 5
    {214, 115, 0, 255},    // 6
    {198, 123, 8, 255},    // 7
    {222, 165, 24, 255},   // 8
    {214, 181, 33, 255},   // 9
    {189, 222, 41, 255},   // 10
    {148, 222, 33, 255},   // 11
    {41, 206, 16, 255},    // 12
    {50, 190, 16, 255},    // 13
    {57, 181, 16, 255},    // 14
    {49, 156, 8, 255},     // 15
    {41, 148, 0, 255},     // 16
    {24, 132, 8, 255},     // 17 = bottom of spec
    {255, 255, 255, 255},  // 18 = osc 1
    {214, 214, 222, 255},  // 19 = osc 2 (slightly dimmer)
    {181, 189, 189, 255},  // 20 = osc 3
    {160, 170, 175, 255},  // 21 = osc 4
    {148, 156, 165, 255},  // 22 = osc 5
    {150, 150, 150, 255}   // 23 = analyzer peak dots
};

Color* osccolors(const Color* colors) {
    static Color osc_colors[16];

    osc_colors[0] = colors[21];
    osc_colors[1] = colors[21];
    osc_colors[2] = colors[20];
    osc_colors[3] = colors[20];
    osc_colors[4] = colors[19];
    osc_colors[5] = colors[19];
    osc_colors[6] = colors[18];
    osc_colors[7] = colors[18];
    osc_colors[8] = colors[19];
    osc_colors[9] = colors[19];
    osc_colors[10] = colors[20];
    osc_colors[11] = colors[20];
    osc_colors[12] = colors[21];
    osc_colors[13] = colors[21];
    osc_colors[14] = colors[22];
    osc_colors[15] = colors[22];

    return osc_colors;
}

void drawRect(SDL_Renderer *renderer, int x, int y, int zoom, Color color) {
    SDL_Rect rect = {x * zoom, y * zoom, zoom, zoom};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void shift_vector_to_right(std::vector<double>& vec) {
    int size = vec.size();
    // Update direction if necessary
    if (SHIFT_AMOUNT >= size - 1 && DIRECTION == 1) {
        // Reached the end, change direction to negative
        DIRECTION = -1;
    } else if (SHIFT_AMOUNT <= 0 && DIRECTION == -1) {
        // Reached the beginning, change direction to positive
        DIRECTION = 1;
    }

    // Update shift amount based on direction
    SHIFT_AMOUNT += DIRECTION;

    // Ensure SHIFT_AMOUNT stays within valid range
    if (SHIFT_AMOUNT >= size) {
        SHIFT_AMOUNT = size - 1;
    } else if (SHIFT_AMOUNT < 0) {
        SHIFT_AMOUNT = 0;
    }

    // Perform rotation based on direction
    if (DIRECTION == 1) {
        std::rotate(vec.rbegin(), vec.rbegin() + 1, vec.rend());
    } else if (DIRECTION == -1) {
        std::rotate(vec.begin(), vec.begin() + 1, vec.end());
    }
    // Uncomment the following line for debugging purposes
    // std::cout << SHIFT_AMOUNT << std::endl;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Winamp Visualization", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * ZOOM, HEIGHT * ZOOM, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Color* osc_colors = osccolors(colors);

    bool sa_thick = true;

    int last_y = 0;
    int top = 0, bottom = 0;
    std::vector<double> sample(75, 0.0); // Initialize vector with 75 elements, all set to 0.0
    sample[0] = 15.0; // Set the first element to 15.0
    static int sapeaks[150];
    static char safalloff[150];
    int sadata2[150];
    static float sadata3[150];
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_b) {
                        // Toggle sa_thick
                        sa_thick = !sa_thick;
                    }
                    break;
                default:
                    break;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        shift_vector_to_right(sample);

        for (int x = 0; x < 76; ++x) {
            for (int y = 0; y < 16; ++y) {
                if (x % 2 == 1 || y % 2 == 0) {
                    Color scope_color = colors[0];
                    drawRect(renderer, x, y, ZOOM, scope_color);
                } else {
                    Color scope_color = colors[1];
                    drawRect(renderer, x, y, ZOOM, scope_color);
                }
            }
        }

/*         for (int x = 0; x < WIDTH; x++) {
            int y = sample[x];

            y = y < 0 ? 0 : (y > HEIGHT - 1 ? HEIGHT - 1 : y);

            if (x == 0) {
                last_y = y;
            }

            top = y;
            bottom = last_y;
            last_y = y;

            if (bottom < top) {
                int temp = bottom;
                bottom = top;
                top = temp + 1;
            }

            int color_index = (top) % 16;
            Color scope_color = osc_colors[color_index];

            for (int dy = top; dy <= bottom; dy++) {
                drawRect(renderer, x, dy, ZOOM, scope_color);
            }
        } */
        for (int x = 0; x < 75; x++) {
        // WHY?! WHY DO I HAVE TO DO THIS?! NOWHERE IS THIS IN THE DECOMPILE
        int i = ((i = x & 0xfffffffc) < 72) ? i : 71; // Limiting i to prevent out of bounds access
            if (sa_thick == true) {
                    // this used to be unoptimized and came straight out of ghidra
                    // here's what that looked like
                    /* uVar12 =  (int)((u_int)*(byte *)(i + 3 + sadata) +
                                    (u_int)*(byte *)(i + 2 + sadata) +
                                    (u_int)*(byte *)(i + 1 + sadata) +
                                    (u_int)*(byte *)(i + sadata)) >> 2; */

                    int uVar12 = static_cast<int>((sample[i+3] + sample[i+2] + sample[i+1] + sample[i]) / 4);

                    // shove the data from uVar12 into sadata2
                    sadata2[x] = uVar12;
            } else if (sa_thick == false) { // just copy sadata to sadata2
                sadata2[x] = sample[x];
            }

        signed char y = safalloff[x];

        safalloff[x] = safalloff[x] - 1;

        // okay this is really funny
        // somehow the internal vis data for winamp/wacup can just, wrap around
        // but here it didnt, until i saw my rect drawing *under* its intended area
        // and i just figured out that winamp's vis box just wraps that around
        // this is really funny to me
        if (sadata2[x] < 0) {
            sadata2[x] = sadata2[x] + 127;
        }
        if (sadata2[x] >= 15) {
            sadata2[x] = 15;
        }

        if (safalloff[x] <= sadata2[x]) {
            safalloff[x] = sadata2[x];
        }

/*
        ghidra output:
        peaks = &DAT_0044de98 + uVar10;
        if (*peaks <= (int)(sum * 0x100)) {
          *peaks = sum * 0x100;
          (&DAT_0044d8c4)[uVar10] = 0x40400000;
        }
*/

    if (sapeaks[x] <= (int)(safalloff[x] * 256)) {
        sapeaks[x] = safalloff[x] * 256;
        sadata3[x] = 3.0f;
    }

    int intValue2 = -(sapeaks[x]/256) + 15;

/*
        ghidra output of winamp 2.63's executable:
        ..
        local_14[0] = 1.05;
        local_14[1] = 1.1;
        local_14[2] = 1.2;
        local_14[3] = 1.4;
        local_14[4] = 1.6;
        ..
        fVar1 = local_14[sum];
        if (DAT_0044de7c == 0) {
            return;
        }
        ..
        lVar20 = __ftol();
        iVar16 = *peaks - (int)lVar20;
        *peaks = iVar16;
        (&DAT_0044d8c4)[uVar10] = fVar1 * (float)(&DAT_0044d8c4)[uVar10];
        if (iVar16 < 0) {
          *peaks = 0;
        }

*/

    sapeaks[x] -= (int)sadata3[x];
    sadata3[x] *= 1.05f;
    if (sapeaks[x] < 0) 
    {
        sapeaks[x] = 0;
    }
        if ((x == i + 3 && x < 72) && sa_thick) {
            // skip rendering
            } else {
                for (int dy = -safalloff[x] + 16; dy <= 17; ++dy) {
                    int color_index = dy + 2; // Assuming dy starts from 0
                    Color scope_color = colors[color_index];
                    drawRect(renderer, x, dy, ZOOM, scope_color); // Assuming drawRect function draws a single pixel
                }            
            }

        if ((x == i + 3 && x < 72) && sa_thick) {
            // skip rendering
            } else {
                if (intValue2 < 15) {
                    Color peaksColor = colors[23];
                    drawRect(renderer, x, intValue2, ZOOM, peaksColor); // Assuming drawRect function draws a single pixel
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}