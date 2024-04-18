#include "headers/Input.h"
#include <string>

const std::unordered_map<std::string, SDL_Scancode> keyMap = {
    // Directional (arrow) Keys
    {"up", SDL_SCANCODE_UP},
    {"down", SDL_SCANCODE_DOWN},
    {"right", SDL_SCANCODE_RIGHT},
    {"left", SDL_SCANCODE_LEFT},

    // Misc Keys
    {"escape", SDL_SCANCODE_ESCAPE},

    // Modifier Keys
    {"lshift", SDL_SCANCODE_LSHIFT},
    {"rshift", SDL_SCANCODE_RSHIFT},
    {"lctrl", SDL_SCANCODE_LCTRL},
    {"rctrl", SDL_SCANCODE_RCTRL},
    {"lalt", SDL_SCANCODE_LALT},
    {"ralt", SDL_SCANCODE_RALT},

    // Editing Keys
    {"tab", SDL_SCANCODE_TAB},
    {"return", SDL_SCANCODE_RETURN},
    {"enter", SDL_SCANCODE_RETURN},
    {"backspace", SDL_SCANCODE_BACKSPACE},
    {"delete", SDL_SCANCODE_DELETE},
    {"insert", SDL_SCANCODE_INSERT},

    // Character Keys
    {"space", SDL_SCANCODE_SPACE},
    {"a", SDL_SCANCODE_A},
    {"b", SDL_SCANCODE_B},
    {"c", SDL_SCANCODE_C},
    {"d", SDL_SCANCODE_D},
    {"e", SDL_SCANCODE_E},
    {"f", SDL_SCANCODE_F},
    {"g", SDL_SCANCODE_G},
    {"h", SDL_SCANCODE_H},
    {"i", SDL_SCANCODE_I},
    {"j", SDL_SCANCODE_J},
    {"k", SDL_SCANCODE_K},
    {"l", SDL_SCANCODE_L},
    {"m", SDL_SCANCODE_M},
    {"n", SDL_SCANCODE_N},
    {"o", SDL_SCANCODE_O},
    {"p", SDL_SCANCODE_P},
    {"q", SDL_SCANCODE_Q},
    {"r", SDL_SCANCODE_R},
    {"s", SDL_SCANCODE_S},
    {"t", SDL_SCANCODE_T},
    {"u", SDL_SCANCODE_U},
    {"v", SDL_SCANCODE_V},
    {"w", SDL_SCANCODE_W},
    {"x", SDL_SCANCODE_X},
    {"y", SDL_SCANCODE_Y},
    {"z", SDL_SCANCODE_Z},
    {"0", SDL_SCANCODE_0},
    {"1", SDL_SCANCODE_1},
    {"2", SDL_SCANCODE_2},
    {"3", SDL_SCANCODE_3},
    {"4", SDL_SCANCODE_4},
    {"5", SDL_SCANCODE_5},
    {"6", SDL_SCANCODE_6},
    {"7", SDL_SCANCODE_7},
    {"8", SDL_SCANCODE_8},
    {"9", SDL_SCANCODE_9},
    {"/", SDL_SCANCODE_SLASH},
    {";", SDL_SCANCODE_SEMICOLON},
    {"=", SDL_SCANCODE_EQUALS},
    {"-", SDL_SCANCODE_MINUS},
    {".", SDL_SCANCODE_PERIOD},
    {",", SDL_SCANCODE_COMMA},
    {"[", SDL_SCANCODE_LEFTBRACKET},
    {"]", SDL_SCANCODE_RIGHTBRACKET},
    {"\\", SDL_SCANCODE_BACKSLASH},
    {"'", SDL_SCANCODE_APOSTROPHE}
};


void Input::Init() {
    // Initialize or reset your input states if necessary
    keyboard_states.clear();
    just_became_down_scancodes.clear();
    just_became_up_scancodes.clear();
    mouse_position = glm::ivec2(0, 0);
    mouse_button_states.clear();
    just_became_down_buttons.clear();
    just_became_up_buttons.clear();
    mouse_scroll_this_frame = 0;
}

void Input::ProcessEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_scancodes.push_back(e.key.keysym.scancode);
        
    }
    else if (e.type == SDL_KEYUP) {
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_scancodes.push_back(e.key.keysym.scancode);
        
    }
    // Handle mouse motion
    else if (e.type == SDL_MOUSEMOTION) {
        mouse_position.x = e.motion.x;
        mouse_position.y = e.motion.y;
    }

    // Handle mouse button down
    else if (e.type == SDL_MOUSEBUTTONDOWN) {
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_buttons.push_back(e.button.button); 
    }

    // Handle mouse button up
    else if (e.type == SDL_MOUSEBUTTONUP) {
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_buttons.push_back(e.button.button); 
    }

    // Handle mouse wheel
    else if (e.type == SDL_MOUSEWHEEL) {
        mouse_scroll_this_frame += e.wheel.preciseY;
    }
}

void Input::LateUpdate() {
    // Update keys that just became down to be in the down state
    for (SDL_Scancode keycode : just_became_down_scancodes) {
        keyboard_states[keycode] = INPUT_STATE_DOWN;
    }
    just_became_down_scancodes.clear();

    // Update keys that just became up to be in the up state
    for (SDL_Scancode keycode : just_became_up_scancodes) {
        keyboard_states[keycode] = INPUT_STATE_UP;
    }
    just_became_up_scancodes.clear();

    // Update mouse buttons that just became down to be in the down state
    for (int button : just_became_down_buttons) {
        mouse_button_states[button] = INPUT_STATE_DOWN;
    }
    just_became_down_buttons.clear();

    // Update mouse buttons that just became up to be in the up state
    for (int button : just_became_up_buttons) {
        mouse_button_states[button] = INPUT_STATE_UP;
    }
    just_became_up_buttons.clear();

    // Reset mouse scroll delta at the end of the frame
    mouse_scroll_this_frame = 0;

}

// Maps string representations to SDL_Scancode values
SDL_Scancode StringToSDLScancode(const std::string& keyString) {
    auto it = keyMap.find(keyString);
    if (it != keyMap.end()) {
        return it->second;
    }
    else {
        return SDL_SCANCODE_UNKNOWN; // Fallback for unknown keys
    }
}

bool Input::GetKey(const std::string& keyString) {
    SDL_Scancode keycode = StringToSDLScancode(keyString);
    if (keycode == SDL_SCANCODE_UNKNOWN) return false;
    return keyboard_states[keycode] == INPUT_STATE_DOWN || keyboard_states[keycode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyDown(const std::string& keyString) {
    SDL_Scancode keycode = StringToSDLScancode(keyString);
    if (keycode == SDL_SCANCODE_UNKNOWN) return false;
    return keyboard_states[keycode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyUp(const std::string& keyString) {
    SDL_Scancode keycode = StringToSDLScancode(keyString);
    if (keycode == SDL_SCANCODE_UNKNOWN) return false;
    return keyboard_states[keycode] == INPUT_STATE_JUST_BECAME_UP;
}

bool Input::GetMouseButton(int button) {
    return mouse_button_states[button] == INPUT_STATE_DOWN || mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonDown(int button) {
    return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonUp(int button) {
    return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_UP;
}

glm::vec2 Input::GetMousePosition() {
    return glm::vec2(mouse_position);
}

float Input::GetMouseScrollDelta() {
    return mouse_scroll_this_frame;
}

