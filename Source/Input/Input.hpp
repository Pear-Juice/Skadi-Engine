#ifndef INPUT_HPP
#define INPUT_HPP


#include <GLFW/glfw3.h>
#include <ankerl/unordered_dense.h>
#include "Source/Resources/Types.hpp"

class Input {
public:
    Input();

    enum Key {
        KEY_NONE = -1,
        KEY_A=65,KEY_B=66,KEY_C=67,KEY_D=68,KEY_E=69,KEY_F=70,KEY_G=71,KEY_H=72,KEY_I=73,KEY_J=74,KEY_K=75,KEY_L=76,KEY_M=77,KEY_N=78,KEY_O=79,KEY_P=80,KEY_Q=81,KEY_R=82,KEY_S=83,KEY_T=84,KEY_U=85,KEY_V=86,KEY_W=87,KEY_X=88,KEY_Y=89,KEY_Z=90,
        NUM_0=48,NUM_1=49,NUM_2=50,NUM_3=51,NUM_4=52,NUM_5=53,NUM_6=54,NUM_7=55,NUM_8=56,NUM_9=57,
        KEY_CTRL=341,KEY_SHIFT=340,KEY_ALT=342,KEY_SPACE=32,KEY_COMMA=44,KEY_PERIOD=46,KEY_FORWARD_SLASH=47,KEY_BACK_SLASH=92,KEY_SEMICOLON=59,KEY_QUOTE=39,KEY_L_BRACKET=91,KEY_R_BRACKET=93,KEY_ENTER,KEY_BACKSPACE=259,KEY_DELETE=261,KEY_DASH=45,KEY_EQUAL=61,KEY_TILDE=96,KEY_ESCAPE=256,KEY_TAB=258,
        LEFT_ARROW=263,DOWN_ARROW=264,UP_ARROW=265,RIGHT_ARROW=262
    };

    enum Mouse {
        MOUSE_NONE=-1,
        MOUSE_1=0, MOUSE_2=1, MOUSE_3=2, MOUSE_4=3, MOUSE_5=4,
        MOUSE_MOVE=5,MOUSE_SCROLL=6,MOUSE_DRAG=7
    };

    enum Mod {MOD_NONE=0, MOD_SHIFT=1, MOD_CTRL=2, MOD_CTRL_SHIFT=3, MOD_ALT=4, MOD_ALT_SHIFT=5, MOD_CTRL_ALT=6};

    enum PressState {
        NONE=-1,JUST_RELEASED=3, RELEASED=1, JUST_PRESSED=12, PRESSED=4
    };

    static std::string keyToString(Key key);
    static std::string modToString(Mod mod);
    static std::string mouseToString(Mouse mouse);
    static std::string pressStateToString(PressState pressState);

    struct KeyData {
        std::string actionName;
        Key key = KEY_NONE;
        Mod mod = MOD_NONE;

        PressState pressState = RELEASED;
        PressState prevPressState = RELEASED;

        std::chrono::time_point<std::chrono::steady_clock> justPressTimestamp;
        std::chrono::time_point<std::chrono::steady_clock> justReleaseTimestamp;
    };

    struct MouseData {
        std::string actionName;
        Mouse button = MOUSE_NONE;
        Mod mod = MOD_NONE;
        PressState pressState = RELEASED;
        PressState prevPressState = RELEASED;

        std::chrono::time_point<std::chrono::steady_clock> justPressTimestamp;
        std::chrono::time_point<std::chrono::steady_clock> justReleaseTimestamp;

        double xPos = 0;
        double yPos = 0;
        double xScroll = 0;
        double yScroll = 0;
    };

    struct KeyMapping {
        std::vector<Key> keys;
        KeyData data;
        std::vector<std::function<void(KeyData)>> callbacks;
    };

    struct MouseMapping {
        std::vector<Mouse> buttons;
        MouseData data;
        std::vector<std::function<void(MouseData)>> callbacks;
    };

    KeyData keyData;
    MouseData mouseData;

    typedef ankerl::unordered_dense::map<std::string,KeyMapping> ActionMap;
    ankerl::unordered_dense::map<std::string, KeyMapping> actions;

    typedef ankerl::unordered_dense::map<std::string,MouseMapping> MouseActionMap;
    MouseActionMap mouseActions;

    void addKeyMapping(std::string name, Key key);
    void addKeyMapping(std::string name, std::vector<Key> keys);
    bool hasKeyMapping(std::string name);
    KeyMapping* getKeyMapping(std::string name, bool updatePressState = true);

    int getKeyAxis(std::string neg, std::string pos, bool updatePressState = true);
    Vector2 getKeyAxis(std::string negX, std::string posX, std::string negY, std::string posY, bool updatePressState = true);

    bool removeKeyMapping(std::string);
    void printKeyMap(const ActionMap& actionMap);

    void addMouseMapping(std::string name, Mouse button);
    void addMouseMapping(std::string name, std::vector<Mouse> buttons);
    bool hasMouseMapping(std::string name);
    MouseMapping* getMouseMapping(std::string, bool updatePressState = true);
    bool removeMouseMapping(std::string);
    void printMouseMap(const MouseActionMap& mouseActionMap);

    void pushKeyCallback(std::string actionName, std::function<void(KeyData)> callback);
    void popKeyCallback(std::string actionName);

    void pushMouseCallback(std::string actionName, std::function<void(MouseData)> callback);
    void popMouseCallback(std::string actionName);

    static std::function<void(Key key, PressState pressState, Mod mod)> keyCallback;
    static std::function<void(Mouse key, PressState pressState, Mod mod)> mouseClickCallback;
    static std::function<void(double xPos, double yPos)> mouseMoveCallback;
    static std::function<void(double xOffset, double yOffset)> mouseScrollCallback;

private:
    void updateCallbacks();
    bool keyLock = false;
    bool mouseLock = false;
    static void processKeyActions(Key key, PressState pressState, Mod mod, ActionMap &actionMap, KeyData &keyData);
    static void processMouseActions(Mouse button, PressState pressState, Mod mod, double xPos, double yPos, std::string type, MouseActionMap &mouseActionMap, MouseData &mouseData);
};

inline std::ostream& operator << (std::ostream& os, Input::Key key) {
     return os << Input::keyToString(key);
}

inline std::ostream& operator << (std::ostream& os, Input::Mod mod) {
     return os << Input::modToString(mod);
}

inline std::ostream& operator << (std::ostream& os, Input::Mouse mouse) {
     return os << Input::mouseToString(mouse);
}

inline std::ostream& operator << (std::ostream& os, Input::PressState& pressState) {
     return os << Input::pressStateToString(pressState);
}

#endif //INPUT_HPP
