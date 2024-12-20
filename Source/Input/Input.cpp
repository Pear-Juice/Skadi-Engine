//
// Created by blankitte on 11/19/24.
//

#include "Input.hpp"

#include <iostream>

std::function<void(Input::Key key, Input::PressState pressState, Input::Mod mod)> Input::keyCallback;
std::function<void(Input::Mouse button, Input::PressState pressState, Input::Mod mod)> Input::mouseClickCallback;
std::function<void(double xPos, double yPos)> Input::mouseMoveCallback;
std::function<void(double xOffset, double yOffset)> Input::mouseScrollCallback;


 void Input::printActionMap(const ActionMap& actionMap) {
    std::cout << "-- Dict --\n";
    for (const Action& action : actionMap) {
        std::cout << "Entry\n";
        std::vector<Key> keys = action.mapping.keys;

        for (int i = 0; i < keys.size(); i++) {
            std::cout << "Name: " << keyToString(keys[i]) << "\n";
        }
    }
}

void Input::printMouseActionMap(const MouseActionMap& actionMap) {
    std::cout << "-- Dict --\n";
    for (const MouseAction& action : actionMap) {
        std::cout << "Entry\n";
        std::vector<Mouse> buttons = action.mapping.buttons;

        for (int i = 0; i < buttons.size(); i++) {
            std::cout << "Name: " << mouseToString(buttons[i]) << "\n";
        }
    }
}

Input::Input() {
     updateCallbacks();
}

void Input::updateCallbacks() {
     keyCallback = [&actionMap = actions, currentKeyData = keyData](const Key key=KEY_NONE, const PressState pressState=RELEASED, const Mod mod=MOD_NONE) mutable
     {processKeyActions(key, pressState, mod, actionMap, currentKeyData);};

     mouseClickCallback = [&mouseActionMap = mouseActions, currentMouseData = &mouseData, type = "Click"](const Mouse button=MOUSE_NONE, const PressState pressState=RELEASED, const Mod mod=MOD_NONE) mutable
     {processMouseActions(button, pressState, mod,currentMouseData->xPos ,currentMouseData->yPos , type, mouseActionMap, *currentMouseData);};

     mouseMoveCallback = [&mouseActionMap = mouseActions, currentMouseData = &mouseData, type = "Move"](const int xPos, const int yPos) mutable
     {processMouseActions(MOUSE_NONE ,RELEASED ,MOD_NONE ,xPos, yPos, type, mouseActionMap, *currentMouseData);};

     mouseScrollCallback = [&mouseActionMap = mouseActions, currentMouseData = &mouseData, type = "Scroll"](const int xOffset, const int yOffset) mutable
     {processMouseActions(MOUSE_NONE, RELEASED, MOD_NONE, xOffset, yOffset, type, mouseActionMap, *currentMouseData);};
 }

Input::Action Input::addAction(std::string name, Key key) {
     // If action list has action, append key to action
     if (hasAction(name)) {
         Action* action = getAction(name);
         action->mapping.keys.push_back(key);
         return *action;
     }

     //Create new action and append key
     KeyMapping mapping{};
     mapping.keys.push_back(key);

     Action action{name, mapping};
     actions.emplace_back(action);

     updateCallbacks();
     return actions.at(actions.size()-1);
}

Input::Action* Input::addAction(std::string name, std::vector<Key> keys) {
     // If action list has action, append key to action
     if (hasAction(name)) {
         Action* action = getAction(name);
         action->mapping.keys.insert(action->mapping.keys.begin(), keys.begin(), keys.end());
         return action;
     }

     //Create new action and append key
     KeyMapping mapping{};
     mapping.keys.insert(mapping.keys.begin(), keys.begin(), keys.end());

     actions.emplace_back(name, mapping);

     updateCallbacks();

     return &actions.back();
 }

Input::MouseAction* Input::addMouseAction(std::string name, Mouse mouse) {
    //If action list has action, append key to action
     if (hasMouseAction(name)) {
         MouseAction* action = getMouseAction(name);
         action->mapping.buttons.push_back(mouse);
         return action;
     }

     //Create new action and append key
     MouseMapping mapping{};
     mapping.buttons.push_back(mouse);

     mouseActions.emplace_back(name, mapping);

     printMouseActionMap(mouseActions);

     updateCallbacks();

     return &mouseActions.back();
 }

Input::MouseAction* Input::addMouseAction(std::string name, std::vector<Mouse> buttons) {
    //If action list has action, append key to action
     if (hasMouseAction(name)) {
         MouseAction* action = getMouseAction(name);
         action->mapping.buttons.insert(action->mapping.buttons.begin(), buttons.begin(), buttons.end());
         return action;
     }

     //Create new action and append key
     MouseMapping mapping{};
     mapping.buttons.insert(mapping.buttons.begin(), buttons.begin(), buttons.end());

     mouseActions.emplace_back(name, mapping);

     updateCallbacks();

     return &mouseActions.back();
 }

bool Input::hasAction(std::string name) {
    for (const Action& action : actions) {
        if (action.name == name) {
            return true;
        }
    }
    return false;
}

bool Input::hasMouseAction(std::string name) {
     for (const MouseAction& mouseAction : mouseActions) {
         if (mouseAction.name == name) {
             return true;
         }
     }
     return false;
 }

Input::Action* Input::getAction(std::string name) {
    for (Action& action : actions) {
        if (action.name == name) {
            return &action;
        }
    }

    return nullptr;
}

Input::MouseAction* Input::getMouseAction(std::string name) {
    for (MouseAction& mouseAction : mouseActions) {
        if (mouseAction.name == name) {
            return &mouseAction;
        }
    }

    return nullptr;
}


bool Input::removeAction(std::string name) {
    std::erase_if( actions, [name](const Action &action){return action.name == name;} );
     updateCallbacks();
    return true;
}

bool Input::removeMouseAction(std::string name) {
    std::erase_if( mouseActions, [name](const MouseAction &mouseAction){return mouseAction.name == name;} );
     updateCallbacks();
    return true;
}

void Input::pushKeyCallback(std::string actionName, std::function<void(KeyData)> callback) {
     if (const auto action = getAction(actionName)) {
         action->mapping.callbacks.push_back(callback);
         updateCallbacks();
     }
     else {
         std::cout << "SKADI: Failed to push callback, matching action does not exist\n";
     }

 }

void Input::popKeyCallback(std::string actionName) {
     if (const auto action = getAction(actionName)) {
         action->mapping.callbacks.pop_back();
         updateCallbacks();
     }
     else {
         std::cout << "SKADI: Failed to pop callback, matching action does not exist\n";
     }
 }


void Input::pushMouseCallback(std::string actionName, std::function<void(MouseData)> callback) {
     if (const auto action = getMouseAction(actionName)) {
         action->mapping.callbacks.push_back(callback);
         updateCallbacks();
     }
     else {
         std::cout << "SKADI: Failed to push callback, matching mouse action does not exist\n";
     }
 }

void Input::popMouseCallback(std::string actionName) {
     if (const auto action = getMouseAction(actionName)) {
         action->mapping.callbacks.pop_back();
         updateCallbacks();
     }
     else {
         std::cout << "SKADI: Failed to pop callback, matching mouse action does not exist\n";
     }
 }

void Input::processKeyActions(Key inputKey, PressState pressState, Mod inputMod, ActionMap &actionMap, KeyData &keyData) {
     KeyData data;
     data.key = inputKey;
     data.pressState = pressState;
     data.mod = inputMod;

    for (auto&[name, mapping] : actionMap) {
        for (Key key : mapping.keys) {
            if (key == inputKey) {
                data.actionName = name;
                mapping.data = data;

                for (const auto& func : mapping.callbacks) {
                    func(data);
                }

                std::cout << "Pressed " << keyToString(mapping.data.key) << " " << mapping.callbacks.size() << "!\n";
            }
        }
    }
      keyData = data;
}

void Input::processMouseActions(Mouse inputButton, PressState pressState, Mod mod, double x, double y, std::string type, MouseActionMap &mouseActionMap, MouseData &mouseData) {
     if (type == "Move") {
         mouseData.button = MOUSE_MOVE;
         mouseData.xPos = x;
         mouseData.yPos = y;

         if (mouseData.pressState == RELEASED) {
             mouseData.mod = MOD_NONE;
         }
         else {
             mouseData.button = MOUSE_DRAG;
         }
     }
     else if (type == "Click") {
         mouseData.pressState = pressState;
         mouseData.button = inputButton;
         mouseData.mod = mod;
     }
     else if (type == "Scroll") {
         mouseData.xScroll = x;
         mouseData.yScroll = y;

         if (mouseData.pressState == RELEASED) {
             mouseData.button = MOUSE_SCROLL;
             mouseData.mod = MOD_NONE;
         }
     }
     // std::cout << " B: " << mouseToString(mouseData.button) << " P: " << mouseData.pressed << " M: " << modToString(mouseData.mod) << " (" << mouseData.xPos << "," << mouseData.yPos << ") (" << mouseData.xScroll << "," << mouseData.yScroll << ")\n";

     for (auto&[name, mapping] : mouseActionMap) {
         for (Mouse button : mapping.buttons) {
             if (button == inputButton) {
                 mouseData.actionName = name;
                 mapping.data = mouseData;

                 for (const auto& func : mapping.callbacks) {
                     func(mouseData);
                 }
             }
         }
         std::cout << "Button " << mapping.data.button << "!\n";
     }

 }

std::string Input::keyToString(Key key) {
    std::string str;

    switch (key) {
        case KEY_NONE: str = "KEY_NONE"; break;
        case NUM_0: str = "NUM_0"; break;
        case NUM_1: str = "NUM_1"; break;
        case NUM_2: str = "NUM_2"; break;
        case NUM_3: str = "NUM_3"; break;
        case NUM_4: str = "NUM_4"; break;
        case NUM_5: str = "NUM_5"; break;
        case NUM_6: str = "NUM_6"; break;
        case NUM_7: str = "NUM_7"; break;
        case NUM_8: str = "NUM_8"; break;
        case NUM_9: str = "NUM_9"; break;

        case KEY_A: str = "KEY_A"; break;
        case KEY_B: str = "KEY_B"; break;
        case KEY_C: str = "KEY_C"; break;
        case KEY_D: str = "KEY_D"; break;
        case KEY_E: str = "KEY_E"; break;
        case KEY_F: str = "KEY_F"; break;
        case KEY_G: str = "KEY_G"; break;
        case KEY_H: str = "KEY_H"; break;
        case KEY_I: str = "KEY_I"; break;
        case KEY_J: str = "KEY_J"; break;
        case KEY_K: str = "KEY_K"; break;
        case KEY_L: str = "KEY_L"; break;
        case KEY_M: str = "KEY_M"; break;
        case KEY_N: str = "KEY_N"; break;
        case KEY_O: str = "KEY_O"; break;
        case KEY_P: str = "KEY_P"; break;
        case KEY_Q: str = "KEY_Q"; break;
        case KEY_R: str = "KEY_R"; break;
        case KEY_S: str = "KEY_S"; break;
        case KEY_T: str = "KEY_T"; break;
        case KEY_U: str = "KEY_U"; break;
        case KEY_V: str = "KEY_V"; break;
        case KEY_W: str = "KEY_W"; break;
        case KEY_X: str = "KEY_X"; break;
        case KEY_Y: str = "KEY_Y"; break;
        case KEY_Z: str = "KEY_Z"; break;
        case KEY_CTRL: str = "KEY_CTRL"; break;
        case KEY_SHIFT: str = "KEY_SHIFT"; break;
        case KEY_ALT: str = "KEY_ALT"; break;
        case KEY_SPACE: str = "KEY_SPACE"; break;
        case KEY_COMMA: str = "KEY_COMMA"; break;
        case KEY_PERIOD: str = "KEY_PERIOD"; break;
        case KEY_FORWARD_SLASH: str = "KEY_FORWARD_SLASH"; break;
        case KEY_BACK_SLASH: str = "KEY_BACK_SLASH"; break;
        case KEY_SEMICOLON: str = "KEY_SEMICOLON"; break;
        case KEY_QUOTE: str = "KEY_QUOTE"; break;
        case KEY_L_BRACKET: str = "KEY_L_BRACKET"; break;
        case KEY_R_BRACKET: str = "KEY_R_BRACKET"; break;
        case KEY_ENTER: str = "KEY_ENTER"; break;
        case KEY_BACKSPACE: str = "KEY_BACKSPACE"; break;
        case KEY_DELETE: str = "KEY_DELETE"; break;
        case KEY_DASH: str = "KEY_DASH"; break;
        case KEY_EQUAL: str = "KEY_EQUAL"; break;
        case KEY_TILDE: str = "KEY_TILDE"; break;
        case KEY_ESCAPE: str = "KEY_ESCAPE"; break;
        case KEY_TAB: str = "KEY_TAB"; break;
        case LEFT_ARROW: str = "LEFT_ARROW"; break;
        case DOWN_ARROW: str = "DOWN_ARROW"; break;
        case UP_ARROW: str = "UP_ARROW"; break;
        case RIGHT_ARROW: str = "RIGHT_ARROW"; break;

        default: str = "Error: Unregistered Key: " + std::to_string(key); break;
    }

    return str;
}

std::string Input::modToString(Mod mod) {
    std::string str;

    switch (mod) {
        case MOD_NONE: str = "MOD_NONE"; break;
        case MOD_SHIFT: str = "MOD_SHIFT"; break;
        case MOD_CTRL: str = "MOD_CTRL"; break;
        case MOD_CTRL_SHIFT: str = "MOD_CTRL_SHIFT"; break;
        case MOD_ALT: str = "MOD_ALT"; break;
        case MOD_ALT_SHIFT: str = "MOD_ALT_SHIFT"; break;
        case MOD_CTRL_ALT: str = "MOD_CTRL_ALT"; break;
        default: str = "Error: Unregistered Mod: " + mod; break;
    }

    return str;
}

std::string Input::mouseToString(Mouse mouse) {
    std::string str;
     switch (mouse) {
         case MOUSE_1: str = "MOUSE_1"; break;
         case MOUSE_2: str = "MOUSE_2"; break;
         case MOUSE_3: str = "MOUSE_3"; break;
         case MOUSE_4: str = "MOUSE_4"; break;
         case MOUSE_5: str = "MOUSE_5"; break;
         case MOUSE_MOVE: str = "MOUSE_MOVE"; break;
         case MOUSE_DRAG: str = "MOUSE_DRAG"; break;
         case MOUSE_SCROLL: str = "MOUSE_SCROLL"; break;
         default: str = "Error: Unregistered Mouse: " + mouse; break;
     }

     return str;
 }

std::string Input::pressStateToString(PressState pressState) {
     std::string str;
     switch (pressState) {
         case RELEASED: str = "RELEASED"; break;
         case PRESSED: str = "PRESSED"; break;
         case HELD: str = "HELD"; break;
         default: str = "Error: Unregistered Press State: " + pressState; break;
     }

     return str;
}



//Input object contains input data
//Input must be received

//What are you doing when you name an event?
//Assigning keys to a name is called an action

//Should you ask if an action is pressed or receive it as an event or both?

//Each action should have a state and a callback

//List of registered actions
//Callback gets called if an action changes value

//Actions need to be retrievable by name


