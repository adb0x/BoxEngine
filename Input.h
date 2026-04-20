#pragma once

#include "SDL.h"
#include "glm/glm.hpp"
#include <unordered_map>
#include <string>
#include <vector>

enum INPUT_STATE { INPUT_STATE_UP, INPUT_STATE_JUST_BECAME_DOWN, INPUT_STATE_DOWN, INPUT_STATE_JUST_BECAME_UP };

class Input {
public:
    static void Init();
    static void ProcessEvent(const SDL_Event& e);
    static void NewFrame(); // Call at the START of your GameLoop

    // --- Keyboard API ---
    static bool GetKey(const std::string& keycode);
    static bool GetKeyDown(const std::string& keycode);
    static bool GetKeyUp(const std::string& keycode);

    // --- Mouse API ---
    static glm::vec2 GetMousePosition() { return mouse_position; }
    static bool GetMouseButton(int button);
    static bool GetMouseButtonDown(int button);
    static bool GetMouseButtonUp(int button);
    static float GetMouseScrollDelta() { return mouse_scroll_delta; }

    static void HideCursor() { SDL_ShowCursor(SDL_DISABLE); }
    static void ShowCursor() { SDL_ShowCursor(SDL_ENABLE); }

    // --- Controller API ---
    // Valid Buttons (string names: "a","b","x","y","start","back",
       //   "leftshoulder","rightshoulder","leftstick","rightstick",
       //   "dpup","dpdown","dpleft","dpright")

    static bool GetControllerButton(const std::string& button);
    static bool GetControllerButtonDown(const std::string& button);
    static bool GetControllerButtonUp(const std::string& button);

    // Axis queries (string names: "leftx","lefty","rightx","righty",
    // "triggerleft","triggerright")
    // Returns float in [-1.0, 1.0]; triggers in [0.0, 1.0]

    static float GetControllerAxis(const std::string& axis);

    // How many controllers are currently connected
    static int GetControllerCount();

    // Rumble: intensity in [0.0, 1.0], duration in milliseconds
    static void RumbleController(float low_freq, float high_freq, int duration_ms);

    static float GetJoystickAxis(int axis_index);
    static bool  GetJoystickButton(int button_index);
    static int GetJoystickAxisCount();
    static int GetJoystickButtonCount();

private:
    // --- Keyboard ---
    static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
    static inline std::vector<SDL_Scancode> just_became_down_scancodes;
    static inline std::vector<SDL_Scancode> just_became_up_scancodes;
    static inline std::unordered_map<std::string, SDL_Scancode> keycode_to_scancode;

    // --- Mouse ---
    static inline std::unordered_map<int, INPUT_STATE> mouse_states;
    static inline std::vector<int> mouse_just_down;
    static inline std::vector<int> mouse_just_up;
    static inline glm::vec2 mouse_position;
    static inline float mouse_scroll_delta = 0;

    // --- Controller ---
    // Keyed by SDL_GameControllerButton (int) -> INPUT_STATE
    static inline std::unordered_map<int, INPUT_STATE> controller_button_states;
    static inline std::vector<int> controller_just_down;
    static inline std::vector<int> controller_just_up;

    // Keyed by SDL_GameControllerAxis (int) -> normalized float value
    static inline std::unordered_map<int, float> controller_axis_values;

    // Open controller handles (supports multiple controllers; rumble uses index 0)
    static inline std::vector<SDL_GameController*> controllers;

    // Helper maps initialized in Init()
    static inline std::unordered_map<std::string, SDL_GameControllerButton> button_name_to_sdl;
    static inline std::unordered_map<std::string, SDL_GameControllerAxis>   axis_name_to_sdl;
};