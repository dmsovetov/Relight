//-----------------------------------------------------------------------------
// Copyright (c) 2007-2008 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#if !defined(INPUT_H)
#define INPUT_H

#if 0

class Keyboard
{
public:
    enum Key
    {
        KEY_0               = 48,
        KEY_1               = 49,
        KEY_2               = 50,
        KEY_3               = 51,
        KEY_4               = 52,
        KEY_5               = 53,
        KEY_6               = 54,
        KEY_7               = 55,
        KEY_8               = 56,
        KEY_9               = 57,

        KEY_A               = 65,
        KEY_B               = 66,
        KEY_C               = 67,
        KEY_D               = 68,
        KEY_E               = 69,
        KEY_F               = 70,
        KEY_G               = 71,
        KEY_H               = 72,
        KEY_I               = 73,
        KEY_J               = 74,
        KEY_K               = 75,
        KEY_L               = 76,
        KEY_M               = 77,
        KEY_N               = 78,
        KEY_O               = 79,
        KEY_P               = 80,
        KEY_Q               = 81,
        KEY_R               = 82,
        KEY_S               = 83,
        KEY_T               = 84,
        KEY_U               = 85,
        KEY_V               = 86,
        KEY_W               = 87,
        KEY_X               = 88,
        KEY_Y               = 89,
        KEY_Z               = 90,

        KEY_F1              = 112,
        KEY_F2              = 113,
        KEY_F3              = 114,
        KEY_F4              = 115,
        KEY_F5              = 116,
        KEY_F6              = 117,
        KEY_F7              = 118,
        KEY_F8              = 119,
        KEY_F9              = 120,
        KEY_F10             = 121,
        KEY_F11             = 122,
        KEY_F12             = 123,

        KEY_BACKSPACE       = 8,
        KEY_TAB             = 9,
        KEY_ENTER           = 13,
        KEY_CAPSLOCK        = 20,
        KEY_ESCAPE          = 27,
        KEY_SPACE           = 32,
        KEY_APP             = 93,
        KEY_SEMICOLON       = 186,
        KEY_ADD             = 187,
        KEY_COMMA           = 188,
        KEY_SUBTRACT        = 189,
        KEY_PERIOD          = 190,
        KEY_FWDSLASH        = 191,
        KEY_TILDE           = 192,
        KEY_LBRACKET        = 219,
        KEY_BACKSLASH       = 220,
        KEY_RBRACKET        = 221,
        KEY_APOSTROPHE      = 222,

        KEY_SHIFT           = 16,
        KEY_CTRL            = 17,
        KEY_LWIN            = 91,
        KEY_RWIN            = 92,
        KEY_LALT            = 164,
        KEY_RALT            = 165,

        KEY_INSERT          = 45,
        KEY_DELETE          = 46,
        KEY_HOME            = 36,
        KEY_END             = 35,
        KEY_PAGEUP          = 33,
        KEY_PAGEDOWN        = 34,

        KEY_UP              = 38,
        KEY_DOWN            = 40,
        KEY_RIGHT           = 39,
        KEY_LEFT            = 37,

        KEY_NUMLOCK         = 144,
        KEY_NUMPAD_DIVIDE   = 111,
        KEY_NUMPAD_MULTIPLY = 106,
        KEY_NUMPAD_SUBTRACT = 109,
        KEY_NUMPAD_ADD      = 107,
        KEY_NUMPAD_DECIMAL  = 110,
        KEY_NUMPAD_0        = 96,
        KEY_NUMPAD_1        = 97,
        KEY_NUMPAD_2        = 98,
        KEY_NUMPAD_3        = 99,
        KEY_NUMPAD_4        = 100,
        KEY_NUMPAD_5        = 101,
        KEY_NUMPAD_6        = 102,
        KEY_NUMPAD_7        = 103,
        KEY_NUMPAD_8        = 104,
        KEY_NUMPAD_9        = 105,
    };

    static Keyboard &instance();

    int getLastChar() const
    { return m_lastChar; }

    bool keyDown(Key key) const
    { return (m_pCurrKeyStates[key] & 0x80) ? true : false; }

    bool keyUp(Key key) const
    { return (m_pCurrKeyStates[key] & 0x80) ? false : true; }

    bool keyPressed(Key key) const
    { 
        return ((m_pCurrKeyStates[key] & 0x80)
            && !(m_pPrevKeyStates[key] & 0x80)) ? true : false;
    }

    void handleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);   
    void update();

private:
    Keyboard();
    ~Keyboard();

    int m_lastChar;
    BYTE m_keyStates[2][256];
    BYTE *m_pCurrKeyStates;
    BYTE *m_pPrevKeyStates;
};


class Mouse
{
public:
    enum MouseButton
    {
        BUTTON_LEFT   = 0,
        BUTTON_RIGHT  = 1,
        BUTTON_MIDDLE = 2
    };

    static Mouse &instance();

    bool buttonDown(MouseButton button) const
    { return m_pCurrButtonStates[button]; }

    bool buttonPressed(MouseButton button) const
    { return m_pCurrButtonStates[button] && !m_pPrevButtonStates[button]; }

    bool buttonUp(MouseButton button) const
    { return !m_pCurrButtonStates[button]; }

    bool cursorIsVisible() const
    { return m_cursorVisible; }

    bool isMouseSmoothing() const
    { return m_enableFiltering; }

    float xDistanceFromWindowCenter() const
    { return m_xDistFromWindowCenter; }

    float yDistanceFromWindowCenter() const
    { return m_yDistFromWindowCenter; }

    int xPos() const
    { return m_ptCurrentPos.x; }

    int yPos() const
    { return m_ptCurrentPos.y; }
        
    float weightModifier() const
    { return m_weightModifier; }

    float wheelPos() const
    { return m_mouseWheel; }

    bool attach(HWND hWnd);
    void detach();
    void handleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void hideCursor(bool hide);
    void moveTo(UINT x, UINT y);
    void moveToWindowCenter();
    void setWeightModifier(float weightModifier);
    void smoothMouse(bool smooth);
    void update();

private:
    Mouse();
    Mouse(const Mouse &);
    Mouse &operator=(const Mouse &);
    ~Mouse();

    void performMouseFiltering(float x, float y);

    static const float DEFAULT_WEIGHT_MODIFIER;
    static const int HISTORY_BUFFER_SIZE = 10;

    HWND m_hWnd;
    int m_historyBufferSize;
    int m_wheelDelta;
    int m_prevWheelDelta;
    float m_mouseWheel;
    float m_xDistFromWindowCenter;
    float m_yDistFromWindowCenter;
    float m_weightModifier;
    float m_filtered[2];
    float m_history[HISTORY_BUFFER_SIZE * 2];
    bool m_moveToWindowCenterPending;
    bool m_enableFiltering;
    bool m_cursorVisible;
    bool m_buttonStates[2][3];
    bool *m_pCurrButtonStates;
    bool *m_pPrevButtonStates;
    POINT m_ptWindowCenterPos;
    POINT m_ptCurrentPos;
};

#endif

#endif