//compile with g++ auto_x_cancel_helper.cpp -o auto_x_cancel_helper -lgdi32
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>

int main() {
    //Makes sure DPI is taken into account
    SetProcessDPIAware();

    std::cout<<"Move your mouse tip to the right most bullet's center. 10 seconds until capture!\n";
    std::this_thread::sleep_for(std::chrono::seconds(10));

    //Gets the mouse's pixel position
    POINT pt;
    GetCursorPos(&pt);
    std::cout<<"xcord is: "<<pt.x<<", ycord is: "<<pt.y<<'\n';

    //gets the color as decimal
    HDC display=GetDC(NULL); //gets full display
    COLORREF color=GetPixel(display, pt.x, pt.y);

    std::cout<<"rgbneed is: "<<color;

    ReleaseDC(NULL, display);

    return 0;
}
