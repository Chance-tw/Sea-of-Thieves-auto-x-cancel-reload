//comile with g++ auto_x_cancel.cpp -o auto_x_cancel -lgdi32
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <windows.h>
#include <atomic>

void killpro(HDC &devicecontext); //kill process

void lisman(std::vector<std::uint8_t> keys, std::atomic<std::uint8_t> &pressedkey); //listener manager
void listener(std::uint8_t key, std::atomic<std::uint8_t> &pressedkey);

bool getrgbvalues(COLORREF rgbneed, int x, int y, HDC &devicecontext);

void sendx();

int main(){
    HDC display=GetDC(NULL); //gets full display as device context

    std::cout<<"CTRL+K to quit the program!";
    //start the listener for ctrl+k to kill the program at any point
    std::thread x(killpro, std::ref(display));
    x.detach();

    /*
    How to use the auto_x_cancel_helper to get info for code to work
        1. Take a screenshot of your game with a weapon out
        2. Open the screenshot in a complete full screen mode
        3. run the auto_x_cancel_helper
        4. After 10 second warning pops up switch to the screenshot window with alt+tab
        5. Put the tip of you mouse over the center of the bullet furthest to the right
        6. After the ten seconds are up, the terminal output will show the color needed for the rgbneed var, and the xcord and ycord
    */

    int xcord=value found with auto_x_cancel_helper;
    int ycord=value found with auto_x_cancel_helper;
    COLORREF rgbneed=value found with auto_x_cancel_helper;
    bool firedgun=false;

    //put hex key codes to keys that switch off your weapons in the liskeys vector seperated by commas replacing the "key", hex key codes found at https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    std::vector<std::uint8_t> liskeys={0x01, key, key}; //0x01=left mouse, needed to determine when gun is fired
    std::atomic<std::uint8_t> pressedkey{0};
    lisman(liskeys, pressedkey); //starts listeners for all the keys to update the pressedkey

    while (true){
        if (getrgbvalues(rgbneed, xcord, ycord, display)){
            if (pressedkey==0x01){ //0x01=left mouse
                firedgun=true;
            }

            if (firedgun){
                firedgun=false;
                pressedkey=0; //flushes pressedkey
                sendx();
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); //small delay between key inputs
                sendx();
            }
            else{
                continue;
            }
        }
        else{
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //small delay between rgb checks
    }
    
    return 0;
}

void killpro(HDC &devicecontext){
    //registers a hotkey to windows for if ctrl+k is pressed with the no repeat modifier
    RegisterHotKey(NULL, 234, 0x0002|0x4000,0x4B); //0x0002=any control, 0x400=no repeat(sending extra hotkey messages past the one), 0x4B=K

    //check if hotkey sent return message from being pressed
    MSG msg={0};
    while (GetMessage(&msg, NULL, 0, 0)){ //when a message recieved
        if (msg.message==WM_HOTKEY){ //if the message is the hotkey
            UnregisterHotKey(NULL, 234); //destroy hotkey so it doesnt persist after program ends
            ReleaseDC(NULL, devicecontext); //release the device context for getting the screen
            exit(0); //kill the whole program
        }
    }
}

void lisman(std::vector<std::uint8_t> keys, std::atomic<std::uint8_t> &pressedkey){
    //spawns all the threads to listen for each possible weapon state changing input, then dies
    for (int i=0; i<(keys.size()); i++){
        std::thread t(listener, keys[i], std::ref(pressedkey));
        t.detach();
    }
}

void listener(std::uint8_t key, std::atomic<std::uint8_t> &pressedkey){
    while (true){ //so the thread will continue and not complete after the input is recieved and pressedkey updated
        while (!(GetAsyncKeyState(key) & 0x8000)){ //while the input is not recieved
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        pressedkey=key; //updates the key pressed
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
