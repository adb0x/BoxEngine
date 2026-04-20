#include <string>
#include <thread>
#include <chrono>
#include "Helper.h" // For GetFrameNumber

class Application {
public:
    static void Quit() {
        exit(0); // Immediate exit as per spec
    }

    static void Sleep(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    static int GetFrame() {
        return Helper::GetFrameNumber();
    }

    static void OpenURL(const std::string& url) {
        std::string command;
#if defined(_WIN32)
        command = "start " + url;
#elif defined(__APPLE__)
        command = "open " + url;
#else
        command = "xdg-open " + url;
#endif
        std::system(command.c_str());
    }
};