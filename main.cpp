#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int main()
{
    // Initialize graphics window
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    initgraph(screenWidth, screenHeight, SHOWCONSOLE);
    BeginBatchDraw();

    // Color scheme
    const COLORREF DARK_BG = RGB(30, 30, 36);
    const COLORREF ACCENT = RGB(0, 145, 255);
    const COLORREF ERROR_RED = RGB(255, 80, 80);
    const COLORREF SUCCESS_GREEN = RGB(80, 255, 80);
    const COLORREF WHITE_TEXT = RGB(240, 240, 240);
    const COLORREF HOVER_COLOR = RGB(100, 100, 110);

    // UI parameters
    const int boxWidth = 400, boxHeight = 50;
    const int btnWidth = 200, btnHeight = 50;
    int x = (screenWidth - boxWidth) / 2;
    int y = screenHeight / 3;

    // Input states
    TCHAR user[16] = {0};
    TCHAR pass[16] = {0};
    bool activeField = false;  // false: username, true: password
    bool inputActive = false;
    clock_t cursorTimer = clock();
    bool showCursor = false;
    bool showError = false;
    bool showSuccess = false;
    bool btnHover = false;
    bool hoverUsername = false;
    bool hoverPassword = false;

    setbkcolor(DARK_BG);
    cleardevice();
    settextstyle(24, 0, _T("Arial"));
    MOUSEMSG m;

    while (true) {
        cleardevice();

        // Draw input fields
        setlinestyle(PS_SOLID, 2);
        
        // Username field
        setlinecolor(hoverUsername ? HOVER_COLOR : (inputActive && !activeField ? ACCENT : WHITE_TEXT));
        rectangle(x, y, x + boxWidth, y + boxHeight);
        
        // Password field
        setlinecolor(hoverPassword ? HOVER_COLOR : (inputActive && activeField ? ACCENT : WHITE_TEXT));
        rectangle(x, y + 80, x + boxWidth, y + 80 + boxHeight);

        // Draw labels
        settextcolor(WHITE_TEXT);
        outtextxy(x - 120, y + 10, _T("Username:"));
        outtextxy(x - 120, y + 90, _T("Password:"));

        // Draw input content
        settextcolor(WHITE_TEXT);
        outtextxy(x + 20, y + 10, user);
        outtextxy(x + 20, y + 90, pass);

        // Draw cursor
        if (clock() - cursorTimer > 500 && inputActive) {
            showCursor = !showCursor;
            cursorTimer = clock();
        }
        if (showCursor && inputActive) {
            int textWidth = activeField ? textwidth(pass) : textwidth(user);
            int baseY = activeField ? y + 90 : y + 10;
            line(x + 20 + textWidth, baseY + 5, x + 20 + textWidth, baseY + 35);
        }

        // Login button (centered)
        int btnX = x + (boxWidth - btnWidth) / 2;
        int btnY = y + 180;
        setlinestyle(PS_SOLID, 2);
        setlinecolor(btnHover ? ACCENT : WHITE_TEXT);
        roundrect(btnX, btnY, btnX + btnWidth, btnY + btnHeight, 10, 10);
        
        // Button text
        settextcolor(WHITE_TEXT);
        int textWidth = textwidth(_T("LOGIN"));
        outtextxy(btnX + (btnWidth - textWidth)/2, btnY + 15, _T("LOGIN"));

        // Status messages
        int msgY = btnY + btnHeight + 30;
        if (showError) {
            settextcolor(ERROR_RED);
            TCHAR* msg = _T("Invalid credentials!");
            outtextxy(screenWidth/2 - textwidth(msg)/2, msgY, msg);
        }
        if (showSuccess) {
            settextcolor(SUCCESS_GREEN);
            TCHAR* msg = _T("Welcome!");
            outtextxy(screenWidth/2 - textwidth(msg)/2, msgY, msg);
        }

        // Mouse handling
        hoverUsername = hoverPassword = btnHover = false;
        if (MouseHit()) {
            m = GetMouseMsg();
            POINT pt = {m.x, m.y};
            
            // Hover detection
            hoverUsername = (pt.x > x && pt.x < x + boxWidth && pt.y > y && pt.y < y + boxHeight);
            hoverPassword = (pt.x > x && pt.x < x + boxWidth && pt.y > y + 80 && pt.y < y + 80 + boxHeight);
            btnHover = (pt.x > btnX && pt.x < btnX + btnWidth && pt.y > btnY && pt.y < btnY + btnHeight);

            if (m.uMsg == WM_LBUTTONDOWN) {
                showError = false;
                showSuccess = false;

                // Handle input field clicks
                if (hoverUsername) {
                    inputActive = true;
                    activeField = false;
                }
                else if (hoverPassword) {
                    inputActive = true;
                    activeField = true;
                }
                // Handle login button click
                else if (btnHover) {
                    if (_tcscmp(user, _T("admin")) == 0 && _tcscmp(pass, _T("123456")) == 0) {
                        showSuccess = true;
                        memset(user, 0, sizeof(user));
                        memset(pass, 0, sizeof(pass));
                    } else {
                        showError = true;
                        memset(user, 0, sizeof(user));
                        memset(pass, 0, sizeof(pass));
                    }
                    inputActive = false;
                }
                else {
                    inputActive = false;
                }

                if (inputActive) {
                    showCursor = true;
                    cursorTimer = clock();
                }
            }
        }

        // Keyboard handling
        if (inputActive && _kbhit()) {
            TCHAR ch = _getch();
            TCHAR* target = activeField ? pass : user;
            size_t len = _tcslen(target);

            switch (ch) {
                case '\b':
                    if (len > 0) target[len-1] = '\0';
                    break;
                case '\t':
                    activeField = !activeField;
                    break;
                case '\r':
                    inputActive = false;
                    break;
                default:
                    if (len < 15 && ch >= 32 && ch <= 126) {
                        target[len] = ch;
                        target[len+1] = '\0';
                    }
            }
            showCursor = true;
            cursorTimer = clock();
        }

        FlushBatchDraw();
        Sleep(20);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
