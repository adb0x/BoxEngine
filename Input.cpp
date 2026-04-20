#include "Input.h"
#include <algorithm>
#include <cmath>

// Axis dead-zone threshold (raw SDL axis value out of 32767)
static constexpr Sint16 AXIS_DEADZONE = 8000;

void Input::Init()
{
    // --- Keyboard ---
    keyboard_states.clear();
    mouse_states.clear();

    keycode_to_scancode = {
        // Directional (arrow) Keys
        {"up", SDL_SCANCODE_UP}, {"down", SDL_SCANCODE_DOWN},
        {"right", SDL_SCANCODE_RIGHT}, {"left", SDL_SCANCODE_LEFT},

        // Misc Keys
        {"escape", SDL_SCANCODE_ESCAPE},

        // Modifier Keys
        {"lshift", SDL_SCANCODE_LSHIFT}, {"rshift", SDL_SCANCODE_RSHIFT},
        {"lctrl", SDL_SCANCODE_LCTRL}, {"rctrl", SDL_SCANCODE_RCTRL},
        {"lalt", SDL_SCANCODE_LALT}, {"ralt", SDL_SCANCODE_RALT},

        // Editing Keys
        {"tab", SDL_SCANCODE_TAB}, {"return", SDL_SCANCODE_RETURN},
        {"enter", SDL_SCANCODE_RETURN}, {"backspace", SDL_SCANCODE_BACKSPACE},
        {"delete", SDL_SCANCODE_DELETE}, {"insert", SDL_SCANCODE_INSERT},

        // Character Keys
        {"space", SDL_SCANCODE_SPACE},
        {"a", SDL_SCANCODE_A}, {"b", SDL_SCANCODE_B}, {"c", SDL_SCANCODE_C}, {"d", SDL_SCANCODE_D},
        {"e", SDL_SCANCODE_E}, {"f", SDL_SCANCODE_F}, {"g", SDL_SCANCODE_G}, {"h", SDL_SCANCODE_H},
        {"i", SDL_SCANCODE_I}, {"j", SDL_SCANCODE_J}, {"k", SDL_SCANCODE_K}, {"l", SDL_SCANCODE_L},
        {"m", SDL_SCANCODE_M}, {"n", SDL_SCANCODE_N}, {"o", SDL_SCANCODE_O}, {"p", SDL_SCANCODE_P},
        {"q", SDL_SCANCODE_Q}, {"r", SDL_SCANCODE_R}, {"s", SDL_SCANCODE_S}, {"t", SDL_SCANCODE_T},
        {"u", SDL_SCANCODE_U}, {"v", SDL_SCANCODE_V}, {"w", SDL_SCANCODE_W}, {"x", SDL_SCANCODE_X},
        {"y", SDL_SCANCODE_Y}, {"z", SDL_SCANCODE_Z},

        {"0", SDL_SCANCODE_0}, {"1", SDL_SCANCODE_1}, {"2", SDL_SCANCODE_2}, {"3", SDL_SCANCODE_3},
        {"4", SDL_SCANCODE_4}, {"5", SDL_SCANCODE_5}, {"6", SDL_SCANCODE_6}, {"7", SDL_SCANCODE_7},
        {"8", SDL_SCANCODE_8}, {"9", SDL_SCANCODE_9},

        // Punctuation
        {"/", SDL_SCANCODE_SLASH}, {";", SDL_SCANCODE_SEMICOLON}, {"=", SDL_SCANCODE_EQUALS},
        {"-", SDL_SCANCODE_MINUS}, {".", SDL_SCANCODE_PERIOD}, {",", SDL_SCANCODE_COMMA},
        {"[", SDL_SCANCODE_LEFTBRACKET}, {"]", SDL_SCANCODE_RIGHTBRACKET},
        {"\\", SDL_SCANCODE_BACKSLASH}, {"'", SDL_SCANCODE_APOSTROPHE}
    };

    // --- Controller button name map ---
    button_name_to_sdl = {
        {"a",            SDL_CONTROLLER_BUTTON_A},
        {"b",            SDL_CONTROLLER_BUTTON_B},
        {"x",            SDL_CONTROLLER_BUTTON_X},
        {"y",            SDL_CONTROLLER_BUTTON_Y},
        {"back",         SDL_CONTROLLER_BUTTON_BACK},
        {"guide",        SDL_CONTROLLER_BUTTON_GUIDE},
        {"start",        SDL_CONTROLLER_BUTTON_START},
        {"leftstick",    SDL_CONTROLLER_BUTTON_LEFTSTICK},
        {"rightstick",   SDL_CONTROLLER_BUTTON_RIGHTSTICK},
        {"leftshoulder", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
        {"rightshoulder",SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
        {"dpup",         SDL_CONTROLLER_BUTTON_DPAD_UP},
        {"dpdown",       SDL_CONTROLLER_BUTTON_DPAD_DOWN},
        {"dpleft",       SDL_CONTROLLER_BUTTON_DPAD_LEFT},
        {"dpright",      SDL_CONTROLLER_BUTTON_DPAD_RIGHT}
    };

    // --- Controller axis name map ---
    axis_name_to_sdl = {
        {"leftx",        SDL_CONTROLLER_AXIS_LEFTX},
        {"lefty",        SDL_CONTROLLER_AXIS_LEFTY},
        {"rightx",       SDL_CONTROLLER_AXIS_RIGHTX},
        {"righty",       SDL_CONTROLLER_AXIS_RIGHTY},
        {"triggerleft",  SDL_CONTROLLER_AXIS_TRIGGERLEFT},
        {"triggerright", SDL_CONTROLLER_AXIS_TRIGGERRIGHT}
    };

    // --- Open any already-connected game controllers ---
    // SDL_INIT_GAMECONTROLLER must have been called in your engine's SDL_Init call.
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            SDL_GameController* ctrl = SDL_GameControllerOpen(i);
            if (ctrl) controllers.push_back(ctrl);
        }
    }
}

// ---------------------------------------------------------------------------
// ProcessEvent
// ---------------------------------------------------------------------------
void Input::ProcessEvent(const SDL_Event& e)
{
    // --- Keyboard ---
    if (e.type == SDL_KEYDOWN)
    {
        SDL_Scancode scancode = e.key.keysym.scancode;
        if (keyboard_states[scancode] != INPUT_STATE_DOWN &&
            keyboard_states[scancode] != INPUT_STATE_JUST_BECAME_DOWN)
        {
            keyboard_states[scancode] = INPUT_STATE_JUST_BECAME_DOWN;
            just_became_down_scancodes.push_back(scancode);
        }
    }
    else if (e.type == SDL_KEYUP)
    {
        SDL_Scancode scancode = e.key.keysym.scancode;
        keyboard_states[scancode] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_scancodes.push_back(scancode);
    }
    // --- Mouse ---
    else if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        int button = e.button.button;
        mouse_states[button] = INPUT_STATE_JUST_BECAME_DOWN;
        mouse_just_down.push_back(button);
    }
    else if (e.type == SDL_MOUSEBUTTONUP)
    {
        int button = e.button.button;
        mouse_states[button] = INPUT_STATE_JUST_BECAME_UP;
        mouse_just_up.push_back(button);
    }
    else if (e.type == SDL_MOUSEMOTION)
    {
        mouse_position.x = static_cast<float>(e.motion.x);
        mouse_position.y = static_cast<float>(e.motion.y);
    }
    else if (e.type == SDL_MOUSEWHEEL)
    {
        mouse_scroll_delta = e.wheel.preciseY;
    }
    // --- Controller hotplug ---
    else if (e.type == SDL_CONTROLLERDEVICEADDED)
    {
        SDL_GameController* ctrl = SDL_GameControllerOpen(e.cdevice.which);
        if (ctrl) controllers.push_back(ctrl);
    }
    else if (e.type == SDL_CONTROLLERDEVICEREMOVED)
    {
        // Find and close the removed controller
        for (auto it = controllers.begin(); it != controllers.end(); ++it)
        {
            SDL_Joystick* joy = SDL_GameControllerGetJoystick(*it);
            if (SDL_JoystickInstanceID(joy) == e.cdevice.which)
            {
                SDL_GameControllerClose(*it);
                controllers.erase(it);
                break;
            }
        }
    }
    // --- Controller buttons ---
    else if (e.type == SDL_CONTROLLERBUTTONDOWN)
    {
        int btn = static_cast<int>(e.cbutton.button);
        if (controller_button_states[btn] != INPUT_STATE_DOWN &&
            controller_button_states[btn] != INPUT_STATE_JUST_BECAME_DOWN)
        {
            controller_button_states[btn] = INPUT_STATE_JUST_BECAME_DOWN;
            controller_just_down.push_back(btn);
        }
    }
    else if (e.type == SDL_CONTROLLERBUTTONUP)
    {
        int btn = static_cast<int>(e.cbutton.button);
        controller_button_states[btn] = INPUT_STATE_JUST_BECAME_UP;
        controller_just_up.push_back(btn);
    }
    // --- Controller axes ---
    else if (e.type == SDL_CONTROLLERAXISMOTION)
    {
        int axis = static_cast<int>(e.caxis.axis);
        Sint16 raw = e.caxis.value;

        // Apply dead-zone: values inside the dead-zone collapse to 0
        float normalized = 0.0f;
        if (std::abs(raw) > AXIS_DEADZONE)
        {
            // Normalize to [-1, 1] (triggers are [0, 32767] -> [0, 1])
            normalized = static_cast<float>(raw) / 32767.0f;
            normalized = std::max(-1.0f, std::min(1.0f, normalized));
        }
        controller_axis_values[axis] = normalized;
    }
}

// ---------------------------------------------------------------------------
// NewFrame  –  call at the START of your game loop, before ProcessEvent
// ---------------------------------------------------------------------------
void Input::NewFrame()
{
    // Keyboard
    for (SDL_Scancode sc : just_became_down_scancodes) keyboard_states[sc] = INPUT_STATE_DOWN;
    for (SDL_Scancode sc : just_became_up_scancodes)   keyboard_states[sc] = INPUT_STATE_UP;
    just_became_down_scancodes.clear();
    just_became_up_scancodes.clear();

    // Mouse
    for (int b : mouse_just_down) mouse_states[b] = INPUT_STATE_DOWN;
    for (int b : mouse_just_up)   mouse_states[b] = INPUT_STATE_UP;
    mouse_just_down.clear();
    mouse_just_up.clear();
    mouse_scroll_delta = 0;

    // Controller buttons
    for (int btn : controller_just_down) controller_button_states[btn] = INPUT_STATE_DOWN;
    for (int btn : controller_just_up)   controller_button_states[btn] = INPUT_STATE_UP;
    controller_just_down.clear();
    controller_just_up.clear();
}

// ---------------------------------------------------------------------------
// Keyboard
// ---------------------------------------------------------------------------
bool Input::GetKey(const std::string& keycode)
{
    if (keycode_to_scancode.find(keycode) == keycode_to_scancode.end()) return false;
    SDL_Scancode sc = keycode_to_scancode[keycode];
    return keyboard_states[sc] == INPUT_STATE_DOWN ||
        keyboard_states[sc] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyDown(const std::string& keycode)
{
    if (keycode_to_scancode.find(keycode) == keycode_to_scancode.end()) return false;
    return keyboard_states[keycode_to_scancode[keycode]] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyUp(const std::string& keycode)
{
    if (keycode_to_scancode.find(keycode) == keycode_to_scancode.end()) return false;
    return keyboard_states[keycode_to_scancode[keycode]] == INPUT_STATE_JUST_BECAME_UP;
}

// ---------------------------------------------------------------------------
// Mouse
// ---------------------------------------------------------------------------
bool Input::GetMouseButton(int button)
{
    int sdl_button = button;
    if (button == 1) sdl_button = SDL_BUTTON_LEFT;
    else if (button == 2) sdl_button = SDL_BUTTON_MIDDLE;
    else if (button == 3) sdl_button = SDL_BUTTON_RIGHT;

    auto it = mouse_states.find(sdl_button);
    if (it == mouse_states.end()) return false;
    return it->second == INPUT_STATE_DOWN || it->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonDown(int button)
{
    return mouse_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonUp(int button)
{
    return mouse_states[button] == INPUT_STATE_JUST_BECAME_UP;
}

// ---------------------------------------------------------------------------
// Controller – buttons
// ---------------------------------------------------------------------------
bool Input::GetControllerButton(const std::string& button)
{
    auto it = button_name_to_sdl.find(button);
    if (it == button_name_to_sdl.end()) return false;
    int btn = static_cast<int>(it->second);
    auto sit = controller_button_states.find(btn);
    if (sit == controller_button_states.end()) return false;
    return sit->second == INPUT_STATE_DOWN || sit->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetControllerButtonDown(const std::string& button)
{
    auto it = button_name_to_sdl.find(button);
    if (it == button_name_to_sdl.end()) return false;
    int btn = static_cast<int>(it->second);
    auto sit = controller_button_states.find(btn);
    if (sit == controller_button_states.end()) return false;
    return sit->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetControllerButtonUp(const std::string& button)
{
    auto it = button_name_to_sdl.find(button);
    if (it == button_name_to_sdl.end()) return false;
    int btn = static_cast<int>(it->second);
    auto sit = controller_button_states.find(btn);
    if (sit == controller_button_states.end()) return false;
    return sit->second == INPUT_STATE_JUST_BECAME_UP;
}

// ---------------------------------------------------------------------------
// Controller – axes
// ---------------------------------------------------------------------------
float Input::GetControllerAxis(const std::string& axis)
{
    auto it = axis_name_to_sdl.find(axis);
    if (it == axis_name_to_sdl.end()) return 0.0f;
    int ax = static_cast<int>(it->second);
    auto ait = controller_axis_values.find(ax);
    if (ait == controller_axis_values.end()) return 0.0f;
    return ait->second;
}

// ---------------------------------------------------------------------------
// Controller – utility
// ---------------------------------------------------------------------------
int Input::GetControllerCount()
{
    return static_cast<int>(controllers.size());
}

void Input::RumbleController(float low_freq, float high_freq, int duration_ms)
{
    if (controllers.empty()) return;
    // SDL rumble takes values in [0, 0xFFFF]
    Uint16 lo = static_cast<Uint16>(std::max(0.0f, std::min(1.0f, low_freq)) * 0xFFFF);
    Uint16 hi = static_cast<Uint16>(std::max(0.0f, std::min(1.0f, high_freq)) * 0xFFFF);
    SDL_GameControllerRumble(controllers[0], lo, hi, static_cast<Uint32>(duration_ms));
}
// ---------------------------------------------------------------------------
// Raw joystick – bypasses GameController abstraction (needed for wheels)
// Uses a much smaller dead-zone (JOY_DEADZONE) so steering wheels and
// pedals are not swallowed at small deflections.
// All four functions are members of Input so they can access private
// members (controllers) directly — no free helper needed.
// ---------------------------------------------------------------------------
float Input::GetJoystickAxis(int axis_index)
{
    SDL_Joystick* joy = nullptr;
    if (!controllers.empty())
        joy = SDL_GameControllerGetJoystick(controllers[0]);
    else if (SDL_NumJoysticks() > 0)
        joy = SDL_JoystickOpen(0);
    if (!joy) return 0.0f;

    Sint16 raw = SDL_JoystickGetAxis(joy, axis_index);
    static constexpr Sint16 JOY_DEADZONE = 800; // ~0.024
    if (std::abs(raw) < JOY_DEADZONE) return 0.0f;
    return std::max(-1.0f, std::min(1.0f, static_cast<float>(raw) / 32767.0f));
}

bool Input::GetJoystickButton(int button_index)
{
    SDL_Joystick* joy = nullptr;
    if (!controllers.empty())
        joy = SDL_GameControllerGetJoystick(controllers[0]);
    else if (SDL_NumJoysticks() > 0)
        joy = SDL_JoystickOpen(0);
    if (!joy) return false;
    return SDL_JoystickGetButton(joy, button_index) == 1;
}

int Input::GetJoystickAxisCount()
{
    SDL_Joystick* joy = nullptr;
    if (!controllers.empty())
        joy = SDL_GameControllerGetJoystick(controllers[0]);
    else if (SDL_NumJoysticks() > 0)
        joy = SDL_JoystickOpen(0);
    if (!joy) return 0;
    return SDL_JoystickNumAxes(joy);
}

int Input::GetJoystickButtonCount()
{
    SDL_Joystick* joy = nullptr;
    if (!controllers.empty())
        joy = SDL_GameControllerGetJoystick(controllers[0]);
    else if (SDL_NumJoysticks() > 0)
        joy = SDL_JoystickOpen(0);
    if (!joy) return 0;
    return SDL_JoystickNumButtons(joy);
}