//compile with g++ auto_x_cancel_helper_linux.cpp -o auto_x_cancel_helper_linux -lX11
#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

struct bulletvars{
    std::uint8_t xcord=0;
    std::uint8_t ycord=0;
    std::uint32_t rgbneed=0;
};

int main(){
    Display *display=XOpenDisplay(NULL); //opens connection to display at the DISPLAY env var
    if (display==NULL){
        std::cout<<"Could not connect to the X server at "<<getenv("DISPLAY")<<"!\n";
        return 0;
    }

    Drawable root = DefaultRootWindow(display); //make a drawable on the root window

    std::cout<<"Move your mouse tip to the right most bullet's center. 10 seconds until capture!\n";
    std::cout<<"Ensure the screenshot is fullscreen!\n";
    std::this_thread::sleep_for(std::chrono::seconds(10));

    //Gets the mouse's pixel position
    Window root_return, child_return;
    int root_x_return, root_y_return, win_x_return, win_y_return;
    unsigned int mousebutton=0;
    XQueryPointer(display, root, &root_return, &child_return, &root_x_return, &root_y_return, &win_x_return, &win_y_return, &mousebutton);
    
    XImage *captured=XGetImage(display, root, root_x_return, root_y_return, 1, 1, AllPlanes, ZPixmap);
    /*
    display: Specifies the connection to the X server.
    root: Specifies where the drawable/window will be
    root_x_return, root_y_return: (x, y) of the top left of rectangle to capture
    1, 1: the size of the rectangle
    AllPlanes: the pixel mask constant to take all the screen
    ZPixmap: the format to return of the pixel map
    */

    //gets the color as decimal
    unsigned long color=XGetPixel(captured, 0, 0);

    std::cout<<"xcord is: "<<root_x_return<<", ycord is: "<<root_y_return<<'\n';
    std::cout<<"rgbneed is: "<<color<<'\n';

    XDestroyImage(captured); //Destroyes the XImage
    XCloseDisplay(display); //closes the connection to the X server

    //populate a struct mirroring the bulletvars struct of the actual macro
    bulletvars cfgfile;
    cfgfile.xcord=root_x_return;
    cfgfile.ycord=root_y_return;
    cfgfile.rgbneed=color;

    //prep for new destination stuff
    char* home=getenv("HOME");
    if (home==NULL){
        std::cout<<"HOME environment variable not set!\n";
        return 0;
    }
    std::string newdirpath=std::string(home)+"/.config/auto_x_cancel";
    std::string newfilepath=newdirpath+"/bullet-config";

    try{
        std::filesystem::create_directories(newdirpath);
    }
    catch(const std::filesystem::filesystem_error& e){ //should catch any errors with writing to the filesystem, but idk how to intentionally cause file system errors
        std::cout<<"Failed to create program directory at "<<newdirpath<<'\n';
        return 0;
    }

    //open/create file to write
    std::ofstream bulletcfg{newfilepath, std::ios::binary};
    if (!bulletcfg){
        std::cout<<"Failed to write to "<<newdirpath<<"/bullet-config\n";
        return 0;
    }

    //write the binary and close
    bulletcfg.write(reinterpret_cast<const char*>(&cfgfile), sizeof(cfgfile));
    bulletcfg.close();

    return 0;
}
