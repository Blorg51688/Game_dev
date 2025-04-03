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

// Global variables for UI state
struct UIState {
    TCHAR user[16];
    TCHAR pass[16];
    bool activeField;      // false: username, true: password
    bool inputActive;
    clock_t cursorTimer;
    bool showCursor;
    bool showError;
    bool showSuccess;
    bool btnHover;
    bool hoverUsername;
    bool hoverPassword;
    int angle;             // Angle for the searchlights
};

// Global variables for UI parameters
struct UIParams {
    int screenWidth;
    int screenHeight;
    int boxWidth;
    int boxHeight;
    int btnWidth;
    int btnHeight;
    int x;
    int y;
    COLORREF DARK_BG;
    COLORREF ACCENT;
    COLORREF ERROR_RED;
    COLORREF SUCCESS_GREEN;
    COLORREF WHITE_TEXT;
    COLORREF HOVER_COLOR;
};

// Function to initialize graphics and UI parameters
UIParams initializeUI() {
    UIParams params;
    params.screenWidth = GetSystemMetrics(SM_CXSCREEN);
    params.screenHeight = GetSystemMetrics(SM_CYSCREEN);
    params.boxWidth = 400;
    params.boxHeight = 50;
    params.btnWidth = 200;
    params.btnHeight = 50;
    params.x = (params.screenWidth - params.boxWidth) / 2;
    params.y = params.screenHeight / 3;
    
    params.DARK_BG = RGB(30, 30, 36);
    params.ACCENT = RGB(0, 145, 255);
    params.ERROR_RED = RGB(255, 80, 80);
    params.SUCCESS_GREEN = RGB(80, 255, 80);
    params.WHITE_TEXT = RGB(240, 240, 240);
    params.HOVER_COLOR = RGB(100, 100, 110);
    
    return params;
}

// Function to initialize UI state
UIState initializeUIState() {
    UIState state = {0};
    state.activeField = false;
    state.inputActive = false;
    state.cursorTimer = clock();
    state.showCursor = false;
    state.showError = false;
    state.showSuccess = false;
    state.btnHover = false;
    state.hoverUsername = false;
    state.hoverPassword = false;
    state.angle = 0;
    return state;
}

// Function to handle keyboard input
void handleKeyboardInput(UIState& state) {
    if (state.inputActive && _kbhit()) {
        TCHAR ch = _getch();
        TCHAR* target = state.activeField ? state.pass : state.user;
        size_t len = _tcslen(target);

        switch (ch) {
            case '\b':
                if (len > 0) target[len - 1] = '\0';
                break;
            case '\t':
                state.activeField = !state.activeField;
                break;
            case '\r':
                state.inputActive = false;
                break;
            default:
                if (len < 15 && ch >= 32 && ch <= 126) {
                    target[len] = ch;
                    target[len + 1] = '\0';
                }
        }
        state.showCursor = true;
        state.cursorTimer = clock();
    }
}

// Function to handle mouse input
void handleMouseInput(UIState& state, const UIParams& params) {
    if (MouseHit()) {
        state.hoverUsername = state.hoverPassword = state.btnHover = false;
        MOUSEMSG m = GetMouseMsg();
        POINT pt = {m.x, m.y};

        // Hover detection
        state.hoverUsername = (pt.x > params.x && pt.x < params.x + params.boxWidth && 
                             pt.y > params.y && pt.y < params.y + params.boxHeight);
        state.hoverPassword = (pt.x > params.x && pt.x < params.x + params.boxWidth && 
                             pt.y > params.y + 80 && pt.y < params.y + 80 + params.boxHeight);
        int btnX = params.x + (params.boxWidth - params.btnWidth) / 2;
        int btnY = params.y + 180;
        state.btnHover = (pt.x > btnX && pt.x < btnX + params.btnWidth && 
                        pt.y > btnY && pt.y < btnY + params.btnHeight);

        if (m.uMsg == WM_LBUTTONDOWN) {
            state.showError = false;
            state.showSuccess = false;

            if (state.hoverUsername) {
                state.inputActive = true;
                state.activeField = false;
            } else if (state.hoverPassword) {
                state.inputActive = true;
                state.activeField = true;
            } else if (state.btnHover) {
                if (checkUserAndPass(state.user, state.pass)) {
                    state.showSuccess = true;
                } else {
                    if (checkUserExist(state.user)) {
                        state.showError = true;
                    } else {
                        if (_tcslen(state.pass) > 0 && registerUser(state.user, state.pass)) {
                            state.showSuccess = true;
                        } else {
                            state.showError = true;
                        }
                    }
                    memset(state.user, 0, sizeof(state.user));
                    memset(state.pass, 0, sizeof(state.pass));
                }
                state.inputActive = false;
            } else {
                state.inputActive = false;
            }

            if (state.inputActive) {
                state.showCursor = true;
                state.cursorTimer = clock();
            }
        }
    }
}

// Function to draw UI elements
void drawUI(UIState& state, const UIParams& params) {
    cleardevice();
    setlinestyle(PS_SOLID, 2);

    // Username field
    setlinecolor(state.inputActive && !state.activeField ? params.ACCENT : 
                (state.hoverUsername ? params.HOVER_COLOR : params.WHITE_TEXT));
    rectangle(params.x, params.y, params.x + params.boxWidth, params.y + params.boxHeight);

    // Password field
    setlinecolor(state.inputActive && state.activeField ? params.ACCENT : 
                (state.hoverPassword ? params.HOVER_COLOR : params.WHITE_TEXT));
    rectangle(params.x, params.y + 80, params.x + params.boxWidth, params.y + 80 + params.boxHeight);

    // Draw labels and input content
    settextcolor(params.WHITE_TEXT);
    outtextxy(params.x - 120, params.y + 10, _T("Username:"));
    outtextxy(params.x - 120, params.y + 90, _T("Password:"));
    outtextxy(params.x + 20, params.y + 10, state.user);
    outtextxy(params.x + 20, params.y + 90, state.pass);

    // Draw cursor
    if (clock() - state.cursorTimer > 500 && state.inputActive) {
        state.showCursor = !state.showCursor;
        state.cursorTimer = clock();
    }
    if (state.showCursor && state.inputActive) {
        int textWidth = state.activeField ? textwidth(state.pass) : textwidth(state.user);
        int baseY = state.activeField ? params.y + 90 : params.y + 10;
        line(params.x + 20 + textWidth, baseY + 5, params.x + 20 + textWidth, baseY + 35);
    }

    // Login button
    int btnX = params.x + (params.boxWidth - params.btnWidth) / 2;
    int btnY = params.y + 180;
    setlinecolor(state.btnHover ? params.ACCENT : params.WHITE_TEXT);
    roundrect(btnX, btnY, btnX + params.btnWidth, btnY + params.btnHeight, 10, 10);

    // Button text
    settextcolor(params.WHITE_TEXT);
    int textWidth = textwidth(_T("LOGIN"));
    outtextxy(btnX + (params.btnWidth - textWidth) / 2, btnY + 15, _T("LOGIN"));

    // Status messages
    int msgY = btnY + params.btnHeight + 30;
    if (state.showError) {
        settextcolor(params.ERROR_RED);
        const TCHAR* msg = _T("Invalid credentials!");
        outtextxy(params.screenWidth / 2 - textwidth(msg) / 2, msgY, msg);
    }
    if (state.showSuccess) {
        settextcolor(params.SUCCESS_GREEN);
        const TCHAR* msg = _T("Welcome!");
        outtextxy(params.screenWidth / 2 - textwidth(msg) / 2, msgY, msg);
        FlushBatchDraw();
        
        // Handle successful login
        FILE* savesFile = _tfopen(_T("saves.txt"), _T("r"));
        if (savesFile) {
            TCHAR line[256];
            while (_fgetts(line, sizeof(line) / sizeof(TCHAR), savesFile)) {
                TCHAR username[16], password[16];
                _stscanf(line, _T("{[%[^,],%[^]]]}"), username, password);
                if (_tcscmp(state.user, username) == 0) {
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

        // Show loading animation
        Sleep(1500);
        int msgX = params.screenWidth / 2 - textwidth(_T("Starting game...")) / 2;
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
    }

    // Draw game elements
    drawAATower(params.screenWidth, params.screenHeight, state.angle);
    drawAABatteries(params.screenWidth, params.screenHeight);
    addAABatteriesProjectiles(params.screenWidth, params.screenHeight);
    simulateFiring();
    
    // Update searchlight angle
    state.angle = (state.angle + 1) % 360;
}

// Main game loop function
bool gameLoop(UIState& state, const UIParams& params) {
    handleMouseInput(state, params);
    handleKeyboardInput(state);
    drawUI(state, params);
    
    FlushBatchDraw();
    Sleep(10);
    
    return !state.showSuccess;
}

int main() {
    // Initialize graphics window and UI parameters
    UIParams params = initializeUI();
    initgraph(params.screenWidth, params.screenHeight, SHOWCONSOLE);
    BeginBatchDraw();

    // Initialize UI state
    UIState state = initializeUIState();

    // Set up graphics settings
    setbkcolor(params.DARK_BG);
    cleardevice();
    settextstyle(24, 0, _T("Arial"));

    // Main game loop
    while (gameLoop(state, params));

    EndBatchDraw();
    closegraph();
    return 0;
}
