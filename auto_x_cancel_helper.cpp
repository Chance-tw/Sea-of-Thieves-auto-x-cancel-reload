//compile with g++ auto_x_cancel_helper.cpp -o auto_x_cancel_helper -lgdi32
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

struct bulletvars{
    std::uint16_t xcord=0;
    std::uint16_t ycord=0;
    std::uint32_t rgbneed=0;
};

int main() {
    //Makes sure DPI is taken into account
    SetProcessDPIAware();

    std::cout<<"Move your mouse tip to the right most bullet's center. 10 seconds until capture!\n";
    std::cout<<"Ensure the screenshot is fullscreen!\n";
    std::this_thread::sleep_for(std::chrono::seconds(10));

    //Gets the mouse's pixel position
    POINT pt;
    GetCursorPos(&pt);
    std::cout<<"xcord is: "<<pt.x<<", ycord is: "<<pt.y<<'\n';

    //gets the color as decimal
    HDC display=GetDC(NULL); //gets full display
    COLORREF color=GetPixel(display, pt.x, pt.y);

    std::cout<<"rgbneed is: "<<color<<'\n';

    ReleaseDC(NULL, display);

    bulletvars cfgfile;
    cfgfile.xcord=pt.x;
    cfgfile.ycord=pt.y;
    cfgfile.rgbneed=color;

    //prep for new destination stuff
    char* home=getenv("HOMEPATH");
    if (home==NULL){
        std::cout<<"HOMEPATH environment variable not set!\n";
        return 0;
    }
    std::string newdirpath=std::string(home)+"/.auto_x_cancel";
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
