#ifndef MINI_GLUT_H
#define MINI_GLUT_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

// Mode constants
#define GLUT_RGB            0
#define GLUT_RGBA           0
#define GLUT_INDEX          1
#define GLUT_SINGLE         0
#define GLUT_DOUBLE         2
#define GLUT_ACCUM          4
#define GLUT_ALPHA          8
#define GLUT_DEPTH          16
#define GLUT_STENCIL        32

// Special keys
#define GLUT_KEY_F1         1
#define GLUT_KEY_F2         2
#define GLUT_KEY_F3         3
#define GLUT_KEY_F4         4
#define GLUT_KEY_F5         5
#define GLUT_KEY_F6         6
#define GLUT_KEY_F7         7
#define GLUT_KEY_F8         8
#define GLUT_KEY_F9         9
#define GLUT_KEY_F10        10
#define GLUT_KEY_F11        11
#define GLUT_KEY_F12        12
#define GLUT_KEY_LEFT       100
#define GLUT_KEY_UP         101
#define GLUT_KEY_RIGHT      102
#define GLUT_KEY_DOWN       103
#define GLUT_KEY_PAGE_UP    104
#define GLUT_KEY_PAGE_DOWN  105
#define GLUT_KEY_HOME       106
#define GLUT_KEY_END        107
#define GLUT_KEY_INSERT     108

#define GLUT_ELAPSED_TIME   700

// Fonts
#define GLUT_BITMAP_9_BY_15        ((void*)1)
#define GLUT_BITMAP_8_BY_13        ((void*)2)
#define GLUT_BITMAP_TIMES_ROMAN_10 ((void*)3)
#define GLUT_BITMAP_TIMES_ROMAN_24 ((void*)4)
#define GLUT_BITMAP_HELVETICA_10   ((void*)5)
#define GLUT_BITMAP_HELVETICA_12   ((void*)6)
#define GLUT_BITMAP_HELVETICA_18   ((void*)7)

namespace MiniGlut {
    inline HWND& getHWnd() { static HWND h = NULL; return h; }
    inline HDC& getHDC() { static HDC h = NULL; return h; }
    inline HGLRC& getHGLRC() { static HGLRC h = NULL; return h; }
    inline int& getInitW() { static int w = 1024; return w; }
    inline int& getInitH() { static int h = 700; return h; }
    inline DWORD& getStartTime() { static DWORD t = 0; return t; }
    inline bool& getRunning() { static bool r = true; return r; }
    inline bool& getNeedRedisplay() { static bool n = true; return n; }

    typedef void (*DisplayFunc)(void);
    typedef void (*ReshapeFunc)(int, int);
    typedef void (*KeyboardFunc)(unsigned char, int, int);
    typedef void (*KeyboardUpFunc)(unsigned char, int, int);
    typedef void (*SpecialFunc)(int, int, int);
    typedef void (*SpecialUpFunc)(int, int, int);
    typedef void (*TimerFunc)(int);

    inline DisplayFunc& getDisplayFunc() { static DisplayFunc f = NULL; return f; }
    inline ReshapeFunc& getReshapeFunc() { static ReshapeFunc f = NULL; return f; }
    inline KeyboardFunc& getKeyboardFunc() { static KeyboardFunc f = NULL; return f; }
    inline KeyboardUpFunc& getKeyboardUpFunc() { static KeyboardUpFunc f = NULL; return f; }
    inline SpecialFunc& getSpecialFunc() { static SpecialFunc f = NULL; return f; }
    inline SpecialUpFunc& getSpecialUpFunc() { static SpecialUpFunc f = NULL; return f; }

    struct TimerEntry {
        DWORD targetTime;
        TimerFunc callback;
        int value;
    };
    inline std::vector<TimerEntry>& getTimers() { static std::vector<TimerEntry> t; return t; }

    // Fonts setup
    struct FontContext {
        GLuint fontBaseHelv18;
        GLuint fontBaseHelv12;
        GLuint fontBaseTimes24;
        HFONT hHelv18;
        HFONT hHelv12;
        HFONT hTimes24;
        bool initialized;
    };
    inline FontContext& getFonts() {
        static FontContext fc = {0, 0, 0, NULL, NULL, NULL, false};
        return fc;
    }

    inline void initFonts(HDC hdc) {
        FontContext& fc = getFonts();
        if (fc.initialized) return;

        fc.fontBaseHelv18 = glGenLists(256);
        fc.hHelv18 = CreateFontA(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Arial");
        SelectObject(hdc, fc.hHelv18);
        wglUseFontBitmapsA(hdc, 0, 256, fc.fontBaseHelv18);

        fc.fontBaseHelv12 = glGenLists(256);
        fc.hHelv12 = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Arial");
        SelectObject(hdc, fc.hHelv12);
        wglUseFontBitmapsA(hdc, 0, 256, fc.fontBaseHelv12);

        fc.fontBaseTimes24 = glGenLists(256);
        fc.hTimes24 = CreateFontA(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Times New Roman");
        SelectObject(hdc, fc.hTimes24);
        wglUseFontBitmapsA(hdc, 0, 256, fc.fontBaseTimes24);

        fc.initialized = true;
    }

    inline LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (getReshapeFunc()) getReshapeFunc()(w, h);
            getNeedRedisplay() = true;
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            if (getDisplayFunc()) getDisplayFunc()();
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_CHAR: {
            if (getKeyboardFunc()) {
                unsigned char c = (unsigned char)wParam;
                getKeyboardFunc()(c, 0, 0);
            }
            return 0;
        }
        case WM_KEYDOWN: {
            int special = 0;
            switch (wParam) {
            case VK_LEFT:  special = GLUT_KEY_LEFT; break;
            case VK_RIGHT: special = GLUT_KEY_RIGHT; break;
            case VK_UP:    special = GLUT_KEY_UP; break;
            case VK_DOWN:  special = GLUT_KEY_DOWN; break;
            default: break;
            }
            if (special && getSpecialFunc()) {
                getSpecialFunc()(special, 0, 0);
            }
            return 0;
        }
        case WM_KEYUP: {
            int special = 0;
            switch (wParam) {
            case VK_LEFT:  special = GLUT_KEY_LEFT; break;
            case VK_RIGHT: special = GLUT_KEY_RIGHT; break;
            case VK_UP:    special = GLUT_KEY_UP; break;
            case VK_DOWN:  special = GLUT_KEY_DOWN; break;
            default: break;
            }
            if (special && getSpecialUpFunc()) {
                getSpecialUpFunc()(special, 0, 0);
            } else if (!special && getKeyboardUpFunc()) {
                // If wParam is printable or standard key
                unsigned char c = (unsigned char)wParam;
                if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
                getKeyboardUpFunc()(c, 0, 0);
            }
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_CLOSE:
            getRunning() = false;
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            getRunning() = false;
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

inline void glutInit(int* /*argc*/, char** /*argv*/) {
    timeBeginPeriod(1);
    MiniGlut::getStartTime() = timeGetTime();
}

inline void glutInitDisplayMode(unsigned int /*mode*/) {}

inline void glutInitWindowSize(int width, int height) {
    MiniGlut::getInitW() = width;
    MiniGlut::getInitH() = height;
}

inline int glutCreateWindow(const char* title) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = MiniGlut::WndProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "MiniGlutWindow";
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    RegisterClassA(&wc);

    RECT r = { 0, 0, MiniGlut::getInitW(), MiniGlut::getInitH() };
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

    int winWidth = r.right - r.left;
    int winHeight = r.bottom - r.top;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - winWidth) / 2;
    int posY = (screenH - winHeight) / 2;
    if (posX < 0) posX = 0;
    if (posY < 0) posY = 0;

    HWND hwnd = CreateWindowExA(0, "MiniGlutWindow", title,
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                posX, posY, winWidth, winHeight,
                                NULL, NULL, GetModuleHandle(NULL), NULL);
    MiniGlut::getHWnd() = hwnd;
    HDC hdc = GetDC(hwnd);
    MiniGlut::getHDC() = hdc;

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, format, &pfd);

    HGLRC hglrc = wglCreateContext(hdc);
    MiniGlut::getHGLRC() = hglrc;
    wglMakeCurrent(hdc, hglrc);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);

    MiniGlut::initFonts(hdc);
    return 1;
}

inline void glutDisplayFunc(void (*func)(void)) { MiniGlut::getDisplayFunc() = func; }
inline void glutReshapeFunc(void (*func)(int, int)) { MiniGlut::getReshapeFunc() = func; }
inline void glutKeyboardFunc(void (*func)(unsigned char, int, int)) { MiniGlut::getKeyboardFunc() = func; }
inline void glutKeyboardUpFunc(void (*func)(unsigned char, int, int)) { MiniGlut::getKeyboardUpFunc() = func; }
inline void glutSpecialFunc(void (*func)(int, int, int)) { MiniGlut::getSpecialFunc() = func; }
inline void glutSpecialUpFunc(void (*func)(int, int, int)) { MiniGlut::getSpecialUpFunc() = func; }
inline void glutIgnoreKeyRepeat(int /*ignore*/) {}

inline void glutPostRedisplay() {
    MiniGlut::getNeedRedisplay() = true;
}

inline void glutSwapBuffers() {
    SwapBuffers(MiniGlut::getHDC());
}

inline int glutGet(int query) {
    if (query == GLUT_ELAPSED_TIME) {
        return (int)(timeGetTime() - MiniGlut::getStartTime());
    }
    return 0;
}

inline void glutTimerFunc(unsigned int millis, void (*func)(int), int val) {
    MiniGlut::TimerEntry entry;
    entry.targetTime = timeGetTime() + millis;
    entry.callback = func;
    entry.value = val;
    MiniGlut::getTimers().push_back(entry);
}

inline void glutMainLoop() {
    MSG msg;
    MiniGlut::getRunning() = true;

    // Initial reshape call
    RECT cr;
    GetClientRect(MiniGlut::getHWnd(), &cr);
    if (MiniGlut::getReshapeFunc()) {
        MiniGlut::getReshapeFunc()(cr.right - cr.left, cr.bottom - cr.top);
    }
    if (MiniGlut::getDisplayFunc()) {
        MiniGlut::getDisplayFunc()();
    }

    while (MiniGlut::getRunning()) {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                MiniGlut::getRunning() = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!MiniGlut::getRunning()) break;

        // Process timers
        DWORD now = timeGetTime();
        std::vector<MiniGlut::TimerEntry>& tList = MiniGlut::getTimers();
        for (size_t i = 0; i < tList.size(); ) {
            if (now >= tList[i].targetTime) {
                MiniGlut::TimerEntry t = tList[i];
                tList.erase(tList.begin() + i);
                if (t.callback) t.callback(t.value);
            } else {
                i++;
            }
        }

        if (MiniGlut::getNeedRedisplay()) {
            MiniGlut::getNeedRedisplay() = false;
            if (MiniGlut::getDisplayFunc()) {
                MiniGlut::getDisplayFunc()();
            }
        }
        Sleep(1);
    }
    timeEndPeriod(1);
}

// 3D Solids
inline void glutSolidSphere(double radius, int slices, int stacks) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
}

inline void glutSolidTorus(double r, double R, int nsides, int rings) {
    const float twoPi = 6.28318530717958647692f;
    for (int i = 0; i < rings; i++) {
        float u0 = (float)i / rings * twoPi;
        float u1 = (float)(i + 1) / rings * twoPi;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= nsides; j++) {
            float v = (float)j / nsides * twoPi;
            float cosV = cos(v), sinV = sin(v);

            float nx0 = cosV * cos(u0);
            float ny0 = cosV * sin(u0);
            float nz0 = sinV;
            glNormal3f(nx0, ny0, nz0);
            glVertex3f((float)((R + r * cosV) * cos(u0)),
                       (float)((R + r * cosV) * sin(u0)),
                       (float)(r * sinV));

            float nx1 = cosV * cos(u1);
            float ny1 = cosV * sin(u1);
            float nz1 = sinV;
            glNormal3f(nx1, ny1, nz1);
            glVertex3f((float)((R + r * cosV) * cos(u1)),
                       (float)((R + r * cosV) * sin(u1)),
                       (float)(r * sinV));
        }
        glEnd();
    }
}

// Text Rendering
inline void glutBitmapCharacter(void* font, int character) {
    MiniGlut::FontContext& fc = MiniGlut::getFonts();
    GLuint base = fc.fontBaseHelv18;
    if (font == GLUT_BITMAP_HELVETICA_12) base = fc.fontBaseHelv12;
    else if (font == GLUT_BITMAP_TIMES_ROMAN_24) base = fc.fontBaseTimes24;

    unsigned char c = (unsigned char)character;
    glListBase(base);
    glCallLists(1, GL_UNSIGNED_BYTE, &c);
}

inline int glutBitmapLength(void* font, const unsigned char* string) {
    if (!string) return 0;
    MiniGlut::FontContext& fc = MiniGlut::getFonts();
    HDC hdc = MiniGlut::getHDC();
    if (!hdc) return (int)strlen((const char*)string) * 10;

    HFONT hFont = fc.hHelv18;
    if (font == GLUT_BITMAP_HELVETICA_12) hFont = fc.hHelv12;
    else if (font == GLUT_BITMAP_TIMES_ROMAN_24) hFont = fc.hTimes24;

    HGDIOBJ old = SelectObject(hdc, hFont);
    SIZE sz = {0, 0};
    GetTextExtentPoint32A(hdc, (const char*)string, (int)strlen((const char*)string), &sz);
    SelectObject(hdc, old);
    return sz.cx;
}

#endif // MINI_GLUT_H
