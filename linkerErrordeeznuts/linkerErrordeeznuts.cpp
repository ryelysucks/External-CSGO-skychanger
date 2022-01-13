#include <iostream>
#include <string>
#include "memory.h"
#include "offsets.h"

std::string originalSky = "";

int option = 0;            //glObAlS bAD!!1

bool enabled = 0;
bool modeVal = 1;

std::string caseValue(int value) //not sure why it's become a hex value....?
{
    switch (value) {
    case 0:
        return originalSky;
    case 2:
        return "jungle";
    case 4:
        return "cs_tibet";
    case 6:
        return "embassy";
    case 8:
        return "italy";
    case 10:
        return "office";
    case 12:
        return "vertigo";
    case 14:
        return "night02";
    case 16:
        return "sky_lunacy";
    case 18:
        return "sky_hr_aztec";
    case 20:
        return "vietnam";
    case 22:
        return "sky_day02_05";
    case 24:
        return "vertigoblue_hdr";
    default:
        return originalSky;
    }
}

void remove3dSky(const auto client, int value) {
    int skyDisable = (Memory("csgo.exe").Read<BYTE>(client + offsets::r3dSky + 0x30));

    if (skyDisable == 57 && value == 1) {
        Memory("csgo.exe").Write<BYTE>(client + offsets::r3dSky + 0x30, skyDisable - 1);
    }
    else if (skyDisable == 56 && value == 0) {
        Memory("csgo.exe").Write<BYTE>(client + offsets::r3dSky + 0x30, skyDisable + 1);
    }
}

void nightmode() {
    const auto mem = Memory("csgo.exe"); //redefining all this stuff made this loop run much faster sorry
    const auto client = mem.GetModuleAddress("client.dll");
    const auto engine = mem.GetModuleAddress("engine.dll");

    const auto localPlayer = mem.Read<std::uintptr_t>(client + offsets::localPlayer);

    float brightness;

    for (int i = 0; i < 1024; i++) {
        const auto entity = mem.Read<std::uintptr_t>(client + offsets::EntityList + i * 0x10);

        if (entity != NULL) {
            int clientNetworktable = mem.Read<int>(entity + 0x8);
            int getClientclass = mem.Read<int>(clientNetworktable + 2 * 0x4);
            int classPointer = mem.Read<int>(getClientclass + 0x1);
            int classID = mem.Read<int>(classPointer + 0x14);

            if (modeVal == 1) {
                brightness = 1.5f;
            }
            else {
                brightness = 0.f;
            }

            const auto thisptr = static_cast<std::uintptr_t>(engine + offsets::modelAmbient - 0x2c);   //adding brightness to random shit...good idea!
            mem.Write<int32_t>(engine + offsets::modelAmbient, *reinterpret_cast<std::uintptr_t*>(&brightness) ^ thisptr);

            if (classID == 69) //nice
            {
                if (modeVal) {
                    mem.Write<BYTE>(entity + offsets::bCustomAutoExposureMin, 1);
                    mem.Write<BYTE>(entity + offsets::bCustomAutoExposureMax, 1);
                    mem.Write<float>(entity + offsets::flCustomAutoExposureMax, 0.065f);
                    mem.Write<float>(entity + offsets::flCustomAutoExposureMin, 0.065f);
                }
                else {
                    mem.Write<BYTE>(entity + offsets::bCustomAutoExposureMin, 1);
                    mem.Write<BYTE>(entity + offsets::bCustomAutoExposureMax, 1);
                    mem.Write<float>(entity + offsets::flCustomAutoExposureMax, 1.f);
                    mem.Write<float>(entity + offsets::flCustomAutoExposureMin, 1.f);
                }
            }
        }
    }
}

bool isPlaying(const auto engine) {
    const auto cState = Memory("csgo.exe").Read<int>(engine + offsets::clientState);
    const auto isplayingVal = Memory("csgo.exe").Read<int>(cState + offsets::clientstateState); //hehe statestate

    if (isplayingVal == 6)
        return true;
    else
        return false;
}

std::string skybox(const auto client) {
    const auto sky = Memory("csgo.exe").Read<std::uintptr_t>(client + offsets::skyboxCV);
    const auto sky2 = Memory("csgo.exe").Read<std::uintptr_t>(sky + 0x8);
    const auto test = sky2 + 0x90;

    if (enabled) {
        if (option == 26)
            option = 0;

        option = option + 1;

        if (caseValue(option) == "night02") {
            char night02[] = { 's', 'k', 'y', '_', 'c', 's', 'g', 'o', '_', 'n', 'i', 'g', 'h', 't', '0', '2', '\0' };  //lol
            Memory("csgo.exe").Write<char[17]>(test, night02);
            remove3dSky(client, 1);
            modeVal = true;
            nightmode();
            return (caseValue(option));
        }
        else if (caseValue(option) == "sky_lunacy") {
            Memory("csgo.exe").Write<std::string>(test, caseValue(option));
            remove3dSky(client, 1);
            modeVal = true;
            nightmode();
            return (caseValue(option));
        }
        else {
            if (caseValue(option) == originalSky) {
                Memory("csgo.exe").Write<std::string>(test, caseValue(option));
                remove3dSky(client, 0);
                modeVal = false;
                nightmode();
                return (caseValue(option) + " (default)");
            }
            else {
                Memory("csgo.exe").Write<std::string>(test, caseValue(option));
                remove3dSky(client, 1);
                modeVal = false;
                nightmode();
                return (caseValue(option));
            }
        }
    }
    else {
        originalSky = Memory("csgo.exe").Read<char[16]>(test);
        return (originalSky + " (default)");
    }
}

void display(const auto client, const auto engine) {
    system("cls");
    std::cout << "Client address found at: " << std::hex << client << std::endl;
    std::cout << "Engine address found at: " << std::hex << engine << std::endl;
    std::cout << "Playing: " << std::boolalpha << isPlaying(engine) << std::endl;
    if (isPlaying(engine))
        std::cout << "Skybox: " << skybox(client) << "\n\n";
}

void doStuff() {
    const auto client = Memory("csgo.exe").GetModuleAddress("client.dll");
    const auto engine = Memory("csgo.exe").GetModuleAddress("engine.dll");
    display(client, engine);

    while (true) {
        Sleep(1);
        if (GetAsyncKeyState(0x70) & 1) {
            enabled = isPlaying(engine);
            skybox(client);
            display(client, engine);
        }
    }
}   

int main() {
    SetConsoleTitle(TEXT("read if cute"));
    HWND console = GetConsoleWindow();
    RECT r;

    GetWindowRect(console, &r);
    MoveWindow(console, r.left, r.top, 400, 200, TRUE);
    ::SetWindowPos(console, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    doStuff();
}
