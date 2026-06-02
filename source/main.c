#include <string.h>
#include <stdio.h>
#include <switch.h>


// Test for Win

int testwin(int a, int b, int c, int points[]) {
    int win= 0;
    if(points[a] == points[b] && points[b] == points[c] && points[a] != 0) {
        points[a] += 3;
        points[b] += 3;
        points[c] += 3;
        win = 1;
    }
    return win;
}

//#define FB_WIDTH  1280
//#define FB_HEIGHT 720

#define FB_WIDTH  1920
#define FB_HEIGHT 1080

// Square with Center coords

bool pointInSquareC(int px, int py, int x, int y, int s) {
    if(px > x-s/2 && x+s/2 > px && y-s/2 < py && y+s/2 > py) {
        return 1;
    }
    return 0;
}


int main(int argc, char **argv)
{

    u32  cnt=0;

    //Enable max-1080p support. Remove for 720p-only resolution.
    //gfxInitResolutionDefault();

    NWindow* win = nwindowGetDefault();
    
    Framebuffer fb;
    framebufferCreate(&fb, win, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2);
    framebufferMakeLinear(&fb);

    #ifdef DISPLAY_IMAGE
        u8* imageptr = (u8*)image_bin;
        const u32 image_width = 1280;
        const u32 image_height = 720;
    #endif


    // Needed Vars
    int player = 1;
    int points[9] = {0,0,0,0,0,0,0,0,0};
    win = 0;

    u32 focused = 3;
    int start = 0;

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    while(appletMainLoop())
    {
        padUpdate(&pad);

        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = padGetButtonsDown(&pad);

        // Check inputs and move focused field/cursor
        if (kDown & HidNpadButton_Plus) break; // break in order to return to hbmenu
        else if (kDown & HidNpadButton_Down) {
            if(focused%3 < 2)
            focused++;
            else focused -= 2;
        }
        else if (kDown & HidNpadButton_Right)  {
            if(focused+3 <= 9) {
                focused += 3;
            } else focused -= 6;
        }
        else if (kDown & HidNpadButton_Left) {
            if(focused-3 >= 0) {
                focused -= 3;
            } else focused += 6;
        }
        else if (kDown & HidNpadButton_Up)  {
            if(focused%3 > 0)
            focused--;
            else focused += 2;
        }
        else if (kDown & HidNpadButton_B) {
            memset(points, 0, sizeof points);
        }
        else if (kDown & HidNpadButton_A) { // Pressed A
            if(points[focused] == 0) { // Set field if field isn't set yet
                points[focused] = player;
                player %= 2;
                player += 1;
            }
            if(win) { // if game's ended, A-Button restarts the game
                memset(points, 0, sizeof points);
                win = 0;
                player = 1;
                focused = 4;
            }
        }
        if(focused > 8 || focused < 0) focused = 4;

        u32 width = FB_WIDTH, height = FB_HEIGHT;
        u32 pos;
        u32 stride;
        u32* framebuf = (u32*) framebufferBegin(&fb, &stride);

        if(start == 0) { // Initialisation
            start = 1;
            focused++; // Somehow not working without this
        }


        if(cnt==15) {
            cnt=0;
        } else {
            cnt++;
        }

        u32 color;

        // Each pixel is 4-bytes due to RGBA8888.
        u32 x, y;
        int s = height/5;
        int b = 5;
        int d = height/5/10;

        //check if someone wins
        if(testwin(0,1,2,points) ||
        testwin(3,4,5,points) ||
        testwin(6,7,8,points) ||
        testwin(0,3,6,points) ||
        testwin(1,4,7,points) ||
        testwin(2,5,8,points) ||
        testwin(0,4,8,points) ||
        testwin(2,4,6,points)) {
            int win = 1;
        }

        // Check if Game ends (all fields are set)
        int un = 1;
        for(int i=0; i<9; i++) {
            if(points[i] == 0) {
                un = 0;
            }
        }
        if(un && win == 1);

        // Setting Win-Color (Changes between blue and turquoise)
        u32 wincolor;
        wincolor = 0xFFFF0000;
        if(cnt > 7) {
            wincolor = 0x33FFF000;
        }

        // If game has ended, no field is focused
        if(win) focused = -1;


        for (y=0; y<height; y++)//Access the buffer linearly.
        {
            for (x=0; x<width; x++)
            {
                pos = y * width + x;
                color = 0x00000000;

                // Background-Color:
                framebuf[pos] = color;

                // Fill Fields
                for(int rx = 0; rx < 3; rx++) {
                    for(int ry = 0; ry < 3; ry++) {
                        int id = ry+rx*3;
                        if(pointInSquareC(x,y, width/2-s-d+(s+d)*rx, height/2-s-d+(s+d)*ry, s+b*2)) {
                            color = 0x00000000;
                            if(focused == id) color = 0xFFFFFFFF; else color = 0x00000000;
                            if(points[id] > 2) color = wincolor;
                            framebuf[pos] = color;//Set framebuf to different shades of grey.
                        }

                        if(pointInSquareC(x,y, width/2-s-d+(s+d)*rx, height/2-s-d+(s+d)*ry, s)) {
                            if(points[id]%3 == 0) color = 0x99999999; // Grey -> Not set
                            if(points[id]%3 == 1) color = 0x1111FFFF; // Player 1 - Color
                            if(points[id]%3 == 2) color = 0x1111FF11; // Player 2 - Color
                            framebuf[pos] = color;//Set framebuf to different shades of grey.
                        }
                    }
                }
            }
        }

        framebufferEnd(&fb);
    }

    framebufferClose(&fb);
    return 0;
}
