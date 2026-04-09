//comile with g++ auto_x_cancel_linux.cpp -o auto_x_cancel_linux -lX11 -lXtst
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <fstream>

std::atomic<bool> running{false};

#pragma pack(1)
struct bulletvars{
    std::uint16_t xcord=0;
    std::uint16_t ycord=0;
    std::uint32_t rgbneed=0;
};

void listener(std::atomic<bool> &firedgun, Display *display, Drawable &root); //listener for left mouse and exit keys

bool getrgbvalues(std::uint32_t rgbneed, int x, int y, Display *display, Drawable &root, XImage *captured);

void sendx(Display *display);

int main(){
    Display *display=XOpenDisplay(NULL); //opens connection to display at the DISPLAY env var
    if (display==NULL){
        std::cout<<"Could not connect to the X server at "<<getenv("DISPLAY")<<"!\n";
        return 0;
    }
    Drawable root = DefaultRootWindow(display); //make a drawable on the root window
    XImage *captured=XGetImage(display, root, 0, 0, 1, 1, AllPlanes, ZPixmap); //initialize ximage structure to have value, so if it gets unallocated it doesnt seg fault
    
    //load bulletvars from /~/.config/auto_x_cancel/bullet-config
    char* home=getenv("HOME");
    if (home==NULL){
        std::cout<<"HOME environment variable not set!\n";
        return 0;
    }
    std::string bulletcfgfile=std::string(home)+"/.config/auto_x_cancel/bullet-config";
    std::ifstream bulletcfg(bulletcfgfile, std::ios::binary);
    if (!bulletcfg){
        std::cout<<"Failed to open /~/.config/auto_x_cancel/bullet-config!\n";
        return 0;
    }

    //populate bullet with data read from the loaded struct
    bulletvars bullet;
    bulletcfg.read(reinterpret_cast<char*>(&bullet), sizeof(bulletvars));
    bulletcfg.close();

    std::cout<<"Left CTRL+K to quit the program!\n";

    /*
    How to use the auto_x_cancel_helper to get info for code to work
        1. Take a screenshot of your game with a weapon out
        2. Open the screenshot in a complete full screen mode
        3. run the auto_x_cancel_helper_linux
        4. After 10 second warning pops up switch to the screenshot window with alt+tab
        5. Put the tip of you mouse over the center of the bullet furthest to the right
    */

    std::atomic<bool> firedgun{false};

    //starts listener for left mouse
    std::thread t(listener, std::ref(firedgun), std::ref(display), std::ref(root));
    t.detach();

    running=true;

    while (running){
        if (getrgbvalues(bullet.rgbneed, bullet.xcord, bullet.ycord, display, root, captured)){
            if (firedgun){ //if the listener updates firedgun from getting the left mouse input
                sendx(display);
                std::this_thread::sleep_for(std::chrono::milliseconds(15)); //small delay between key inputs
                sendx(display);
            }
            else{
                continue;
            }
        }
        
        firedgun=false; //reset firedgun outside if to remove it staying true after last bullet fired, causing the input to send when ammo is gotten from having none
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //small delay between rgb checks
    }

    if (captured!=NULL){
        XDestroyImage(captured); //Destroyes the XImage
    }
    XCloseDisplay(display); //stop connection to x server
    return 0;
}

void listener(std::atomic<bool> &firedgun, Display *display, Drawable &root){
    Window root_return, child_return; //disregarded XQueryPointer returns
    int root_x_return, root_y_return, win_x_return, win_y_return; //disregarded XQueryPointer returns
    std::uint32_t mousebutton=0; //the mask returned by XQueryPointer

    KeyCode kc = XKeysymToKeycode(display, XK_k); //translates k to keycode
    KeyCode ctrl = XKeysymToKeycode(display, XK_Control_L); //translates left ctrl to keycode
    char keys_return[32]; //to hold key states

    bool run=true;

    while (run){
        //get mouse state
        XQueryPointer(display, root, &root_return, &child_return, &root_x_return, &root_y_return, &win_x_return, &win_y_return, &mousebutton);
        
        XQueryKeymap(display, keys_return); //gets key states

        if (mousebutton & Button1Mask){ //bitwise to compare only the bit that matters to the left mouse button being down
            firedgun=true;
        }
        //if left ctrl+k are held
        else if ((keys_return[kc / 8] & (1 << (kc % 8))) && (keys_return[ctrl / 8] & (1 << (ctrl % 8)))){
            run=false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    running=false; //update running to stop main thread from viewing XImage again
}

bool getrgbvalues(std::uint32_t rgbneed, int x, int y, Display *display, Drawable &root, XImage *captured){
    captured=XGetImage(display, root, x, y, 1, 1, AllPlanes, ZPixmap);
    /*
    display: Specifies the connection to the X server.
    root: Specifies where the drawable/window will be
    1770, 980: (x, y) of the top left of rectangle to capture
    1, 1: the size of the rectangle
    AllPlanes: the pixel mask constant to take all the screen
    ZPixmap: the format to return of the pixel map
    */

    if (captured!=NULL){
        std::uint32_t rgb=XGetPixel(captured, 0, 0); //gets the pixel of the captured XImage

        if (rgbneed==rgb){ //if the rgb is the bullet color
            return true;
        }
    }

    return false;
}

void sendx(Display *display){
    //TestFakeKeyEvent(Display *display, unsigned int keycode, Bool is_press, unsigned long delay);
    KeyCode xkey=XKeysymToKeycode(display, XK_x); //gets keycode of x key

    //key down
    XTestFakeKeyEvent(display, xkey, True, 0);
    
    //key up
    XTestFakeKeyEvent(display, xkey, False, 0);
}
