#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

static inline int macroIndexFromAction(GpioAction a) {
    if (a >= GpioAction::BUTTON_PRESS_MACRO_1 && a <= GpioAction::BUTTON_PRESS_MACRO_6)
        return (int)a - (int)GpioAction::BUTTON_PRESS_MACRO_1;        // 0..5
    if (a >= GpioAction::BUTTON_PRESS_MACRO_7 && a <= GpioAction::BUTTON_PRESS_MACRO_12)
        return 6 + ((int)a - (int)GpioAction::BUTTON_PRESS_MACRO_7);  // 6..11
    return -1;
}

bool InputMacro::available() {
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        GpioAction act = pinMappings[pin].action;
        if (act == GpioAction::BUTTON_PRESS_MACRO) return true;   // 共通トリガーボタン
        if (macroIndexFromAction(act) >= 0) return true;           // Macro 1..12 のどれか
    }
    return false;
}

void InputMacro::setup() {
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    macroButtonMask = 0;
    memset(macroPinMasks, 0, sizeof(macroPinMasks));

    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        GpioAction act = pinMappings[pin].action;
        if (act == GpioAction::BUTTON_PRESS_MACRO) {
            macroButtonMask |= (1u << pin);
        } else {
            int idx = macroIndexFromAction(act);
            if (idx >= 0 && idx < MAX_MACRO_LIMIT)
                macroPinMasks[idx] |= (1u << pin);
        }
    }

    inputMacroOptions = &Storage::getInstance().getAddonOptions().macroOptions;
    if (inputMacroOptions->macroBoardLedEnabled && isValidPin(BOARD_LED_PIN)) {
        gpio_init(BOARD_LED_PIN);
        gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
        boardLedEnabled = true;
    } else {
        boardLedEnabled = false;
    }
    // ↓これが残っていると常に消灯になるので削除/コメントアウト推奨
    // boardLedEnabled = false;

    prevMacroInputPressed = false;
    reset();
}

    inputMacroOptions = &Storage::getInstance().getAddonOptions().macroOptions;
    if (inputMacroOptions->macroBoardLedEnabled && isValidPin(BOARD_LED_PIN)) {
        gpio_init(BOARD_LED_PIN);
        gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
        boardLedEnabled = true;
    } else {
        boardLedEnabled = false;
    }
    boardLedEnabled = false;
    prevMacroInputPressed = false;
    reset();
}


void InputMacro::reset() {
    macroPosition = -1;
    pressedMacro = -1;
    isMacroRunning = false;
    macroStartTime = 0;
    macroInputPosition = 0;
    isMacroTriggerHeld = false;
    macroInputHoldTime = INPUT_HOLD_US;
    if (boardLedEnabled) {
        gpio_put(BOARD_LED_PIN, 0);
    }
}

void InputMacro::restart(Macro& macro) {
    macroStartTime = currentMicros;
    macroInputPosition = 0;
    MacroInput& newMacroInput = macro.macroInputs[macroInputPosition];
    uint32_t newMacroInputDuration = newMacroInput.duration + newMacroInput.waitDuration;
    macroInputHoldTime = newMacroInputDuration <= 0 ? INPUT_HOLD_US : newMacroInputDuration;
}

void InputMacro::checkMacroPress() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t allPins = gamepad->debouncedGpio;

    // Go through our macro list
    pressedMacro = -1;
    for(int i = 0; i < MAX_MACRO_LIMIT; i++) {
        if ( inputMacroOptions->macroList[i].enabled == false ) // Skip disabled macros
            continue;
        Macro * macro = &inputMacroOptions->macroList[i];
        if ( macro->useMacroTriggerButton ) {
            // Use Gamepad Button for Macro Trigger
            if ((allPins & macroButtonMask) &&
                ((gamepad->state.buttons & macro->macroTriggerButton) ||
                    (gamepad->state.dpad & (macro->macroTriggerButton >> 16))) ) {
                pressedMacro = i;
                break;
            }
        } else if ( allPins & macroPinMasks[i] ) {
            // Use Pin Manager for Macro Trigger
            pressedMacro = i;
            break;
        }
    }
}

void InputMacro::checkMacroAction() {
    bool macroInputPressed = (pressedMacro != -1); // Was any macro input pressed?

    // Is our pressed macro button different from our current macro AND no macro is running?
    if ( pressedMacro != macroPosition && !isMacroRunning ) {
        macroPosition = pressedMacro; // move our position to that macro
    }

    bool newPress = macroInputPressed && (prevMacroInputPressed ^ macroInputPressed);

    // Check to see if we should change the current macro (or turn off based on input)
    if ( inputMacroOptions->macroList[macroPosition].macroType == ON_PRESS ) {
        // START Macro: On Press or On Hold Repeat
        if (!isMacroRunning ) {
            isMacroTriggerHeld = newPress;
        }
    } else if ( inputMacroOptions->macroList[macroPosition].macroType == ON_HOLD_REPEAT ) {
        isMacroTriggerHeld = macroInputPressed;
    } else if ( inputMacroOptions->macroList[macroPosition].macroType == ON_TOGGLE ) {
        //isMacroTriggerHeld = macroInputPressed;
        if (!isMacroRunning ) {
            isMacroTriggerHeld = newPress;
        } else if (isMacroRunning && newPress) {
            // STOP Macro: Toggle on new press
            reset(); // Stop Macro: Toggle
            prevMacroInputPressed = macroInputPressed;
            return;
        }
    }

    prevMacroInputPressed = macroInputPressed;
    if (!isMacroRunning && isMacroTriggerHeld) {
        // New Macro to run
        macroPosition = pressedMacro; // Set current macro
        Macro& macro = inputMacroOptions->macroList[macroPosition];
        MacroInput& macroInput = macro.macroInputs[macroInputPosition];
        uint32_t macroInputDuration = macroInput.duration + macroInput.waitDuration;
        macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_US : macroInputDuration;
        isMacroRunning = true;
        macroStartTime = getMicro(); // current time
    }
}

void InputMacro::runCurrentMacro() {
    // Do nothing if macro is not currently running
    if (!isMacroRunning ||
            macroPosition == -1)
        return;

    Macro& macro = inputMacroOptions->macroList[macroPosition];

    // Stop Macro if released (ON PRESS & ON HOLD REPEAT)
    if (inputMacroOptions->macroList[macroPosition].macroType == ON_HOLD_REPEAT &&
            !isMacroTriggerHeld ) {
        reset();
        return;
    }

    MacroInput& macroInput = macro.macroInputs[macroInputPosition];
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    currentMicros = getMicro();

    if (!macro.interruptible && macro.exclusive) {
        // Prevent any other inputs from modifying our input (Exclusive)
        gamepad->state.dpad = 0;
        gamepad->state.buttons = 0;
    } else {
        if (macro.useMacroTriggerButton) {
            // Remove the trigger button from the input state
            gamepad->state.dpad &= ~(macro.macroTriggerButton >> 16);
            gamepad->state.buttons &= ~macro.macroTriggerButton;
        }
        if (macro.interruptible &&
            (gamepad->state.buttons != 0 || gamepad->state.dpad != 0)) {
            // Macro is interruptible and a user pressed something
            reset();
            return;
        }
    }

    // Have we elapsed the input hold time?
    if ((currentMicros - macroStartTime) >= macroInputHoldTime) {
        macroStartTime = currentMicros;
        macroInputPosition++;
        
        if (macroInputPosition >= (macro.macroInputs_count)) {
            if ( macro.macroType == ON_PRESS ) {
                reset(); // On press = no more macro
            } else {
                restart(macro); // On Hold-Repeat or On Toggle = start macro again
            }
        } else {
            MacroInput& newMacroInput = macro.macroInputs[macroInputPosition];
            uint32_t newMacroInputDuration = newMacroInput.duration + newMacroInput.waitDuration;
            macroInputHoldTime = newMacroInputDuration <= 0 ? INPUT_HOLD_US : newMacroInputDuration;
        }
    }

    // Check if we should still hold this macro input based on duration
    if ((currentMicros - macroStartTime) <= macroInput.duration) {
        uint32_t buttonMask = macroInput.buttonMask;
        if (buttonMask & GAMEPAD_MASK_DU) {
            gamepad->state.dpad |= GAMEPAD_MASK_UP;
        }
        if (buttonMask & GAMEPAD_MASK_DD) {
            gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
        }
        if (buttonMask & GAMEPAD_MASK_DL) {
            gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
        }
        if (buttonMask & GAMEPAD_MASK_DR) {
            gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
        }
        gamepad->state.buttons |= buttonMask;

        // Macro LED is on if we're currently running and inputs are doing something (wait-timers turn it off)
        if (boardLedEnabled) {
            gpio_put(BOARD_LED_PIN, (gamepad->state.dpad || gamepad->state.buttons) ? 1 : 0);
        }
    }
}

void InputMacro::preprocess()
{
    FocusModeOptions * focusModeOptions = &Storage::getInstance().getAddonOptions().focusModeOptions;
    if (focusModeOptions->enabled && focusModeOptions->macroLockEnabled)
        return;

    checkMacroPress();
    checkMacroAction();
    runCurrentMacro();
}

void InputMacro::reinit() {
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    macroButtonMask = 0;
    memset(macroPinMasks, 0, sizeof(macroPinMasks));

    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        GpioAction act = pinMappings[pin].action;
        if (act == GpioAction::BUTTON_PRESS_MACRO) {
            macroButtonMask |= (1u << pin);
        } else {
            int idx = macroIndexFromAction(act);
            if (idx >= 0 && idx < MAX_MACRO_LIMIT)
                macroPinMasks[idx] |= (1u << pin);
        }
    }
}
