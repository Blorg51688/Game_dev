#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <bits/stdc++.h>

using namespace std;

// Function to check if the user exists and the password matches
bool checkUserAndPass(const TCHAR* user, const TCHAR* pass) {
    FILE* file = _tfopen(_T("saves.txt"), _T("r"));
    if (!file) return false;

    TCHAR line[256], username[16], password[16];
    bool userExists = false, passwordMatch = false;

    while (_fgetts(line, sizeof(line) / sizeof(TCHAR), file)) {
        _stscanf(line, _T("{[%[^,],%[^]]]}"), username, password);
        if (_tcscmp(user, username) == 0) {
            userExists = true;
            if (_tcscmp(pass, password) == 0) {
                passwordMatch = true;
            }
            break;
        }
    }

    fclose(file);
    return userExists && passwordMatch;
}

// Function to check if the user exists
bool checkUserExist(const TCHAR* user) {
    FILE* file = _tfopen(_T("saves.txt"), _T("r"));
    if (!file) return false;

    TCHAR line[256], username[16];
    while (_fgetts(line, sizeof(line) / sizeof(TCHAR), file)) {
        _stscanf(line, _T("{[%[^,],%*[^]]]}"), username);
        if (_tcscmp(user, username) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

// Function to register a new user
bool registerUser(const TCHAR* user, const TCHAR* pass) {
    if (_tcslen(pass) == 0) return false; // Password cannot be empty

    // Check if the user already exists
    if (checkUserExist(user)) {
        return false; // User already exists, do not register again
    }

    FILE* file = _tfopen(_T("saves.txt"), _T("a"));
    if (!file) return false;

    _ftprintf(file, _T("{[%s,%s],[0,0,0],[0,0,0,0,0]}\n"), user, pass);
    fclose(file);
    return true;
}

// Function to store projectile positions
struct Projectile {
    int x;
    int y;
    int life; // Number of frames that the projectile will remain active
    float angle; // Angle for projectile direction
    clock_t fireTime; // Time when the projectile was fired
};

// Store projectiles
vector<Projectile> projectiles;
// Track last firing time for each anti-aircraft battery
clock_t lastFiringTimeBattery1 = 0;
clock_t lastFiringTimeBattery2 = 0;

// Function to add new projectiles from anti-aircraft batteries
void addAABatteriesProjectiles(int screenWidth, int screenHeight) {
    int battery1X = screenWidth / 4;
    int battery2X = screenWidth * 3 / 4;
    int batteryY = screenHeight - 30;

    // Check if battery 1 is ready to fire
    if (clock() - lastFiringTimeBattery1 > rand() % 3000 + 1000) { // Random cooldown
        // Calculate firing angle for battery 1
        float angle1 = 45.0f + static_cast<float>(rand() % 90); // Angle between 30 and 60 degrees

        // Create projectiles for battery 1
        for (int i = 0; i < (rand()%8)+2 ; ++i) {
            Projectile p1;
            p1.x = battery1X + (rand() % 20 - 10);
            p1.y = batteryY - 20; // Start position above battery
            p1.life = 2000; // Projectile lifetime
            p1.angle = angle1; // Store firing angle
            p1.fireTime = clock() + i * 100; // Staggered firing timing
            projectiles.push_back(p1);
        }

        lastFiringTimeBattery1 = clock(); // Update last firing time
    }

    // Check if battery 2 is ready to fire
    if (clock() - lastFiringTimeBattery2 > rand() % 3000 + 1000) { // Random cooldown
        // Calculate firing angle for battery 2
        float angle2 = 45.0f + static_cast<float>(rand() % 90); // Angle between 30 and 60 degrees

        // Create projectiles for battery 2
        for (int i = 0; i < (rand()%8)+2 ; ++i) {
            Projectile p2;
            p2.x = battery2X + (rand() % 20 - 10);
            p2.y = batteryY - 20; // Start position above battery
            p2.life = 2000; // Projectile lifetime
            p2.angle = angle2; // Store firing angle
            p2.fireTime = clock() + i * 100; // Staggered firing timing
            projectiles.push_back(p2);
        }

        lastFiringTimeBattery2 = clock(); // Update last firing time
    }
}



// Function to simulate firing from anti-aircraft batteries
void simulateFiring() {
    // Update and render projectiles
    setfillcolor(RGB(255, 0, 0)); // Bright red projectile
    double pSpeed = 10.0; // Projectile speed
    clock_t currentTime = clock(); // Get the current time once

    for (auto it = projectiles.begin(); it != projectiles.end();) {
        if (currentTime >= it->fireTime && it->life > 0) {  // Check if it's time to fire the projectile
            solidcircle(it->x, it->y, 3); // Draw projectile
            it->y -= pSpeed * sin(it->angle * 3.14159265 / 180); // Move the projectile based on its angle
            it->x += pSpeed * cos(it->angle * 3.14159265 / 180); // Move to the direction of the angle
            it->life--; // Decrease life
            ++it;
        } else if (it->life <= 0) {
            it = projectiles.erase(it); // Remove expired projectiles
        } else {
            ++it; // Move to the next projectile without updating position yet
        }
    }
}



// Function to draw the anti-aircraft tower
void drawAATower(int screenWidth, int screenHeight, int angle) {
    int baseX = screenWidth / 2;
    int baseY = screenHeight - 30;
    int towerHeight = 80; // Reduced tower height

    // Draw the main tower
    setlinecolor(RGB(255, 255, 255));
    rectangle(baseX - 50, baseY, baseX + 50, baseY - towerHeight);

    rectangle(baseX - 70, baseY - towerHeight - 20, baseX - 30, baseY - towerHeight + 20);
    rectangle(baseX - 60, baseY - towerHeight - 10, baseX - 40, baseY - towerHeight + 10);
    rectangle(baseX - 100, baseY - 20, baseX - 20, baseY);

    rectangle(baseX + 70, baseY - towerHeight - 20, baseX + 30, baseY - towerHeight + 20);
    rectangle(baseX + 60, baseY - towerHeight - 10, baseX + 40, baseY - towerHeight + 10);
    rectangle(baseX + 100, baseY - 20, baseX + 20, baseY);

    // Draw the top lights
    int light1X1 = baseX - 50;
    int light1Y1 = baseY - towerHeight;
    int light1X2 = light1X1 + 2000 * cos((abs(45 - (0.25 * ((angle) % 360))) + 90) * 3.14159265 / 180); // Extend to screen edge
    int light1Y2 = light1Y1 - 2000 * sin((abs(45 - (0.25 * ((angle) % 360))) + 90) * 3.14159265 / 180); // Extend to screen edge

    int light2X1 = baseX + 50;
    int light2Y1 = baseY - towerHeight;
    int light2X2 = light2X1 + 2000 * cos((abs(45 - (0.25 * ((angle + 36) % 360))) + 45) * 3.14159265 / 180); // Extend to screen edge
    int light2Y2 = light2Y1 - 2000 * sin((abs(45 - (0.25 * ((angle + 36) % 360))) + 45) * 3.14159265 / 180); // Extend to screen edge

    setlinecolor(RGB(255, 255, 0)); // Yellow light
    line(light1X1, light1Y1, light1X2, light1Y2);
    line(light2X1, light2Y1, light2X2, light2Y2);
}

// Function to draw anti-aircraft batteries
void drawAABatteries(int screenWidth, int screenHeight) {
    int battery1X = screenWidth / 4;
    int battery2X = screenWidth * 3 / 4;
    int batteryY = screenHeight - 30;
    int batteryWidth = 20;
    int batteryHeight = 40; // Reduced battery height

    setlinecolor(RGB(255, 255, 255));

    // Draw the first battery
    rectangle(battery1X - 5, batteryY, battery1X + 5, batteryY - 50);
    rectangle(battery1X - 30, batteryY, battery1X + 30, batteryY - 20);
    rectangle(battery1X - 40, batteryY, battery1X + 40, batteryY - 10);

    // Draw the second battery
    rectangle(battery2X - 5, batteryY, battery2X + 5, batteryY - 50);
    rectangle(battery2X - 30, batteryY, battery2X + 30, batteryY - 20);
    rectangle(battery2X - 40, batteryY, battery2X + 40, batteryY - 10);
}

int main() {
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

    int angle = 0; // Angle for the searchlights
    clock_t lastFiringTime = 0; // Time for the last firing from batteries

    while (true) {
        cleardevice();

        // Draw input fields
        setlinestyle(PS_SOLID, 2);

        // Username field
        setlinecolor(inputActive && !activeField ? ACCENT : (hoverUsername ? HOVER_COLOR : WHITE_TEXT));
        rectangle(x, y, x + boxWidth, y + boxHeight);

        // Password field
        setlinecolor(inputActive && activeField ? ACCENT : (hoverPassword ? HOVER_COLOR : WHITE_TEXT));
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
        outtextxy(btnX + (btnWidth - textWidth) / 2, btnY + 15, _T("LOGIN"));

        // Status messages
        int msgY = btnY + btnHeight + 30;
        if (showError) {
            settextcolor(ERROR_RED);
            const TCHAR* msg = _T("Invalid credentials!");
            outtextxy(screenWidth / 2 - textwidth(msg) / 2, msgY, msg);
        }
        if (showSuccess) {
            settextcolor(SUCCESS_GREEN);
            const TCHAR* msg = _T("Welcome!");
            outtextxy(screenWidth / 2 - textwidth(msg) / 2, msgY, msg);
            FlushBatchDraw();
            // Read user data from saves.txt and write to activeUser.txt
            FILE* savesFile = _tfopen(_T("saves.txt"), _T("r"));
            if (savesFile) {
                TCHAR line[256];
                while (_fgetts(line, sizeof(line) / sizeof(TCHAR), savesFile)) {
                    TCHAR username[16], password[16];
                    _stscanf(line, _T("{[%[^,],%[^]]]}"), username, password);
                    if (_tcscmp(user, username) == 0) {
                        FILE* activeUserFile = _tfopen(_T("activeUser.txt"), _T("w"));
                        if (activeUserFile) {
                            _ftprintf(activeUserFile, _T("%s"), line);
                            fclose(activeUserFile);
                        }
                        break;
                    }
                }
                fclose(savesFile);
            }
            Sleep(1500);
            int msgX = screenWidth / 2 - textwidth(_T("Starting game...")) / 2;
            msg = _T("Starting game.");
            outtextxy(msgX, msgY, msg);
            FlushBatchDraw();
            Sleep(500);
            msg = _T("Starting game..");
            outtextxy(msgX, msgY, msg);
            FlushBatchDraw();
            Sleep(500);
            msg = _T("Starting game...");
            outtextxy(msgX, msgY, msg);
            FlushBatchDraw();
            system("start gameCore.exe");
            Sleep(500);
            EndBatchDraw();
            closegraph();
            return 0;
        }

        // Mouse handling
        if (MouseHit()) {
            hoverUsername = hoverPassword = btnHover = false;
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
                } else if (hoverPassword) {
                    inputActive = true;
                    activeField = true;
                } else if (btnHover) {
                    if (checkUserAndPass(user, pass)) {
                        showSuccess = true;
                    } else {
                        // Check if user exists
                        if (checkUserExist(user)) {
                            showError = true; // User exists but password doesn't match
                        } else {
                            // User doesn't exist, try to register
                            if (_tcslen(pass) > 0 && registerUser(user, pass)) {
                                showSuccess = true;
                            } else {
                                showError = true; // Empty password or registration failed
                            }
                        }
                        memset(user, 0, sizeof(user));
                        memset(pass, 0, sizeof(pass));
                    }
                    inputActive = false;
                } else {
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
                    if (len > 0) target[len - 1] = '\0';
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
                        target[len + 1] = '\0';
                    }
            }
            showCursor = true;
            cursorTimer = clock();
        }

        // Draw anti-aircraft tower and searchlights
        drawAATower(screenWidth, screenHeight, angle);

        // Draw anti-aircraft batteries
        drawAABatteries(screenWidth, screenHeight);

        // Add new projectiles
        addAABatteriesProjectiles(screenWidth, screenHeight);

        // Simulate firing
        simulateFiring();

        // Update the angle for the searchlights
        angle = (angle + 1) % 360;

        FlushBatchDraw();
        Sleep(10); // Slower the firing animation
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
