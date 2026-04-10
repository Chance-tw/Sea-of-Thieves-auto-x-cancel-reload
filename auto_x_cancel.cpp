//comile with g++ auto_x_cancel.cpp -o auto_x_cancel -lgdi32
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <windows.h>
#include <atomic>
#include <fstream>

#pragma pack(1)
struct bulletvars{
    std::uint16_t xcord=0;
    std::uint16_t ycord=0;
    std::uint32_t rgbneed=0;
};

void killpro(std::atomic<bool> &killing); //kill process

void listener(std::atomic<bool> &firedgun, std::atomic<bool> &killing); //listener for left mouse

bool getrgbvalues(COLORREF rgbneed, int x, int y, HDC &devicecontext);

void sendx();

int main(){
    HDC display=GetDC(NULL); //gets full display as device context

    std::cout<<"CTRL+K to quit the program!";
    //start the listener for ctrl+k to kill the program at any point
    std::atomic<bool> killing{false}; //to notify threads to end
    std::thread x(killpro, std::ref(killing));
    x.detach();

    /*
    How to use the auto_x_cancel_helper to get info for code to work
        1. Take a screenshot of your game with a weapon out
        2. Open the screenshot in a complete full screen mode
        3. run the auto_x_cancel_helper.exe
        4. After 10 second warning pops up switch to the screenshot window with alt+tab
        5. Put the tip of you mouse over the center of the bullet furthest to the right
    */

    //load bulletvars from /~/.auto_x_cancel/bullet-config
    char* home=getenv("HOMEPATH");
    if (home==NULL){
        std::cout<<"HOMEPATH environment variable not set!\n";
        return 0;
    }
    std::string bulletcfgfile=std::string(home)+"/.auto_x_cancel/bullet-config";
    std::ifstream bulletcfg(bulletcfgfile, std::ios::binary);
    if (!bulletcfg){
        std::cout<<"Failed to open "<<home<<"/.auto_x_cancel/bullet-config!\n";
        return 0;
    }

    //populate bullet with data read from the loaded struct
    bulletvars bullet;
    bulletcfg.read(reinterpret_cast<char*>(&bullet), sizeof(bulletvars));
    bulletcfg.close();

    std::atomic<bool> firedgun{false};

    //starts listener for left mouse
    std::thread t(listener, std::ref(firedgun), std::ref(killing));
    t.detach();

    while (!killing){
        if (getrgbvalues(bullet.rgbneed, bullet.xcord, bullet.ycord, display)){
            if (firedgun){ //if the listener updates firedgun from getting the left mouse input
                sendx();
                std::this_thread::sleep_for(std::chrono::milliseconds(15)); //small delay between key inputs
                sendx();
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); //larger delay to prevent excess x inputs
            }
            else{
                continue;
            }
        }
        
        firedgun=false; //reset firedgun outside if to remove it staying true after last bullet fired, causing the input to send when ammo is gotten from having none
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //small delay between rgb checks
    }

    ReleaseDC(NULL, display); //release the device context for getting the screen
            
    return 0;
}

void killpro(std::atomic<bool> &killing){
    //registers a hotkey to windows for if ctrl+k is pressed with the no repeat modifier
    RegisterHotKey(NULL, 234, 0x0002|0x4000,0x4B); //0x0002=any control, 0x400=no repeat(sending extra hotkey messages past the one), 0x4B=K

    //check if hotkey sent return message from being pressed
    MSG msg={0};
    while (GetMessage(&msg, NULL, 0, 0)){ //when a message recieved
        if (msg.message==WM_HOTKEY){ //if the message is the hotkey
            UnregisterHotKey(NULL, 234); //destroy hotkey so it doesnt persist after program ends
            killing=true;
        }
    }
}

void listener(std::atomic<bool> &firedgun, std::atomic<bool> &killing){
    while (!killing){ //so the thread will continue and not complete after the input is recieved and pressedkey up
        while (!(GetAsyncKeyState(0x01) & 0x8000) && !killing){ //while the input 0x01(left mouse) is not recieved
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        firedgun=true; //updates if input is recieved and gets past the busy loop
    }
}

bool getrgbvalues(COLORREF rgbneed, int x, int y, HDC &devicecontext){
    COLORREF rgb=GetPixel(devicecontext, x, y); //gets the rgb at (x, y)

    if (rgbneed==rgb){ //if the rgb is the bullet color
        return true;
    }

    return false;
}

void sendx(){
    INPUT input[2] = {}; //input event

    //key down
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 'X';
    input[0].ki.dwFlags = 0;

    //key up
    input[1] = input[0];
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, input, sizeof(INPUT)); //sends the input event
}
