#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* 检查用户名和密码是否匹配 */
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

/* 检查用户是否存在 */
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

/* 注册新用户 */
bool registerUser(const TCHAR* user, const TCHAR* pass) {
    if (_tcslen(pass) == 0) return false; /* 密码不能为空 */

    /* 检查用户是否已存在 */
    if (checkUserExist(user)) {
        return false; /* 用户已存在，不再注册 */
    }

    FILE* file = _tfopen(_T("saves.txt"), _T("a"));
    if (!file) return false;

    _ftprintf(file, _T("{[%s,%s],[0,0,0],[0,0,0,0,0]}\n"), user, pass);
    fclose(file);

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
    return true;
}

/* 定义炮弹结构体 */
typedef struct {
    int x;
    int y;
    int life;       /* 炮弹存活的帧数 */
    float angle;    /* 炮弹方向角度 */
    clock_t fireTime; /* 炮弹发射时间 */
} Projectile;

/* 炮弹数组及其管理 */
#define MAX_PROJECTILES 200
Projectile projectiles[MAX_PROJECTILES];
int projectileCount = 0;

/* 电池上次发射时间 */
clock_t lastFiringTimeBattery1 = 0;
clock_t lastFiringTimeBattery2 = 0;

/* 添加炮弹到数组 */
void addProjectile(int x, int y, float angle, clock_t fireTime) {
    if (projectileCount < MAX_PROJECTILES) {
        projectiles[projectileCount].x = x;
        projectiles[projectileCount].y = y;
        projectiles[projectileCount].life = 2000;
        projectiles[projectileCount].angle = angle;
        projectiles[projectileCount].fireTime = fireTime;
        projectileCount++;
    }
}

/* 从防空炮台添加炮弹 */
void addAABatteriesProjectiles(int screenWidth, int screenHeight) {
    int battery1X = screenWidth / 4;
    int battery2X = screenWidth * 3 / 4;
    int batteryY = screenHeight - 30;

    /* 检查炮台1是否准备好发射 */
    if (clock() - lastFiringTimeBattery1 > rand() % 3000 + 1000) { /* 随机冷却时间 */
        /* 计算炮台1的发射角度 */
        float angle1 = 45.0f + (float)(rand() % 90); /* 角度在45到135度之间 */

        /* 为炮台1创建炮弹 */
        int projectileCount = (rand() % 8) + 2;
        int i;
        for (i = 0; i < projectileCount; ++i) {
            int projX = battery1X + (rand() % 20 - 10);
            int projY = batteryY - 20; /* 炮弹起始位置在炮台上方 */
            addProjectile(projX, projY, angle1, clock() + i * 100); /* 错开发射时间 */
        }

        lastFiringTimeBattery1 = clock(); /* 更新上次发射时间 */
    }

    /* 检查炮台2是否准备好发射 */
    if (clock() - lastFiringTimeBattery2 > rand() % 3000 + 1000) { /* 随机冷却时间 */
        /* 计算炮台2的发射角度 */
        float angle2 = 45.0f + (float)(rand() % 90); /* 角度在45到135度之间 */

        /* 为炮台2创建炮弹 */
        int projectileCount = (rand() % 8) + 2;
        int i;
        for (i = 0; i < projectileCount; ++i) {
            int projX = battery2X + (rand() % 20 - 10);
            int projY = batteryY - 20; /* 炮弹起始位置在炮台上方 */
            addProjectile(projX, projY, angle2, clock() + i * 100); /* 错开发射时间 */
        }

        lastFiringTimeBattery2 = clock(); /* 更新上次发射时间 */
    }
}

/* 模拟炮弹发射 */
void simulateFiring(void) {
    /* 更新并渲染炮弹 */
    setfillcolor(RGB(255, 0, 0)); /* 亮红色炮弹 */
    double pSpeed = 10.0; /* 炮弹速度 */
    clock_t currentTime = clock(); /* 获取当前时间 */
    int i, j;

    for (i = 0; i < projectileCount; i++) {
        if (currentTime >= projectiles[i].fireTime && projectiles[i].life > 0) {
            solidcircle(projectiles[i].x, projectiles[i].y, 3); /* 绘制炮弹 */
            projectiles[i].y -= pSpeed * sin(projectiles[i].angle * 3.14159265 / 180); /* 根据角度移动炮弹 */
            projectiles[i].x += pSpeed * cos(projectiles[i].angle * 3.14159265 / 180); /* 向角度方向移动 */
            projectiles[i].life--; /* 减少生命值 */
        } else if (projectiles[i].life <= 0) {
            /* 移除过期炮弹 - 通过将数组中最后一个元素移到当前位置 */
            for (j = i; j < projectileCount - 1; j++) {
                projectiles[j] = projectiles[j + 1];
            }
            projectileCount--;
            i--; /* 因为当前位置现在有一个新元素，所以需要重新检查 */
        }
    }
}

/* 绘制防空塔 */
void drawAATower(int screenWidth, int screenHeight, int angle) {
    int baseX = screenWidth / 2;
    int baseY = screenHeight - 30;
    int towerHeight = 80; /* 减小塔高 */

    /* 绘制主塔 */
    setlinecolor(RGB(255, 255, 255));
    rectangle(baseX - 50, baseY, baseX + 50, baseY - towerHeight);

    rectangle(baseX - 70, baseY - towerHeight - 20, baseX - 30, baseY - towerHeight + 20);
    rectangle(baseX - 60, baseY - towerHeight - 10, baseX - 40, baseY - towerHeight + 10);
    rectangle(baseX - 100, baseY - 20, baseX - 20, baseY);

    rectangle(baseX + 70, baseY - towerHeight - 20, baseX + 30, baseY - towerHeight + 20);
    rectangle(baseX + 60, baseY - towerHeight - 10, baseX + 40, baseY - towerHeight + 10);
    rectangle(baseX + 100, baseY - 20, baseX + 20, baseY);

    /* 绘制顶部灯光 */
    int light1X1 = baseX - 50;
    int light1Y1 = baseY - towerHeight;
    int light1X2 = light1X1 + 2000 * cos((abs(45 - (0.25 * ((angle) % 360))) + 90) * 3.14159265 / 180); /* 延伸到屏幕边缘 */
    int light1Y2 = light1Y1 - 2000 * sin((abs(45 - (0.25 * ((angle) % 360))) + 90) * 3.14159265 / 180); /* 延伸到屏幕边缘 */

    int light2X1 = baseX + 50;
    int light2Y1 = baseY - towerHeight;
    int light2X2 = light2X1 + 2000 * cos((abs(45 - (0.25 * ((angle + 36) % 360))) + 45) * 3.14159265 / 180); /* 延伸到屏幕边缘 */
    int light2Y2 = light2Y1 - 2000 * sin((abs(45 - (0.25 * ((angle + 36) % 360))) + 45) * 3.14159265 / 180); /* 延伸到屏幕边缘 */

    setlinecolor(RGB(255, 255, 0)); /* 黄色灯光 */
    line(light1X1, light1Y1, light1X2, light1Y2);
    line(light2X1, light2Y1, light2X2, light2Y2);
}

/* 绘制防空炮台 */
void drawAABatteries(int screenWidth, int screenHeight) {
    int battery1X = screenWidth / 4;
    int battery2X = screenWidth * 3 / 4;
    int batteryY = screenHeight - 30;
    int batteryWidth = 20;
    int batteryHeight = 40; /* 减小炮台高度 */

    setlinecolor(RGB(255, 255, 255));

    /* 绘制第一个炮台 */
    rectangle(battery1X - 5, batteryY, battery1X + 5, batteryY - 50);
    rectangle(battery1X - 30, batteryY, battery1X + 30, batteryY - 20);
    rectangle(battery1X - 40, batteryY, battery1X + 40, batteryY - 10);

    /* 绘制第二个炮台 */
    rectangle(battery2X - 5, batteryY, battery2X + 5, batteryY - 50);
    rectangle(battery2X - 30, batteryY, battery2X + 30, batteryY - 20);
    rectangle(battery2X - 40, batteryY, battery2X + 40, batteryY - 10);
}

/* UI状态结构体 */
typedef struct {
    TCHAR user[16];
    TCHAR pass[16];
    bool activeField;      /* false: 用户名, true: 密码 */
    bool inputActive;
    clock_t cursorTimer;
    bool showCursor;
    bool showError;
    bool showSuccess;
    bool btnHover;
    bool hoverUsername;
    bool hoverPassword;
    int angle;             /* 探照灯角度 */
    bool showHelp;         /* 是否显示帮助界面 */
} UIState;

/* UI参数结构体 */
typedef struct {
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
} UIParams;

/* 初始化图形和UI参数 */
UIParams initializeUI(void) {
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

/* 初始化UI状态 */
UIState initializeUIState(void) {
    UIState state;
    memset(&state, 0, sizeof(UIState));
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
    state.showHelp = false; /* 初始化为不显示帮助界面 */
    return state;
}

/* 处理键盘输入 */
void handleKeyboardInput(UIState* state) {
    if (_kbhit()) {
        TCHAR ch = _getch();
        
        /* 检测H键切换帮助界面 */
        if (ch == 'h' || ch == 'H') {
            if (!state->inputActive) { /* 只有在非输入状态下才切换帮助界面 */
                state->showHelp = !state->showHelp;
                return;
            }
        }
        
        if (state->inputActive) {
            TCHAR* target = state->activeField ? state->pass : state->user;
            size_t len = _tcslen(target);

            switch (ch) {
                case '\b':
                    if (len > 0) target[len - 1] = '\0';
                    break;
                case '\t':
                    state->activeField = !state->activeField;
                    break;
                case '\r':
                    state->inputActive = false;
                    break;
                default:
                    if (len < 15 && ch >= 32 && ch <= 126) {
                        target[len] = ch;
                        target[len + 1] = '\0';
                    }
            }
            state->showCursor = true;
            state->cursorTimer = clock();
        }
    }
}

/* 处理鼠标输入 */
void handleMouseInput(UIState* state, const UIParams* params) {
    /* 帮助界面显示时不处理鼠标输入 */
    if (state->showHelp) return;
    
    if (MouseHit()) {
        state->hoverUsername = state->hoverPassword = state->btnHover = false;
        MOUSEMSG m = GetMouseMsg();
        POINT pt = {m.x, m.y};

        /* 悬停检测 */
        state->hoverUsername = (pt.x > params->x && pt.x < params->x + params->boxWidth && 
                             pt.y > params->y && pt.y < params->y + params->boxHeight);
        state->hoverPassword = (pt.x > params->x && pt.x < params->x + params->boxWidth && 
                             pt.y > params->y + 80 && pt.y < params->y + 80 + params->boxHeight);
        int btnX = params->x + (params->boxWidth - params->btnWidth) / 2;
        int btnY = params->y + 180;
        state->btnHover = (pt.x > btnX && pt.x < btnX + params->btnWidth && 
                        pt.y > btnY && pt.y < btnY + params->btnHeight);

        if (m.uMsg == WM_LBUTTONDOWN) {
            state->showError = false;
            state->showSuccess = false;

            if (state->hoverUsername) {
                state->inputActive = true;
                state->activeField = false;
            } else if (state->hoverPassword) {
                state->inputActive = true;
                state->activeField = true;
            } else if (state->btnHover) {
                if (checkUserAndPass(state->user, state->pass)) {
                    state->showSuccess = true;
                } else {
                    if (checkUserExist(state->user)) {
                        state->showError = true;
                    } else {
                        if (_tcslen(state->pass) > 0 && registerUser(state->user, state->pass)) {
                            state->showSuccess = true;
                        } else {
                            state->showError = true;
                        }
                    }
                    memset(state->user, 0, sizeof(state->user));
                    memset(state->pass, 0, sizeof(state->pass));
                }
                state->inputActive = false;
            } else {
                state->inputActive = false;
            }

            if (state->inputActive) {
                state->showCursor = true;
                state->cursorTimer = clock();
            }
        }
    }
}

/* 绘制UI元素 */
void drawUI(UIState* state, const UIParams* params) {
    cleardevice();
    setlinestyle(PS_SOLID, 2);

    /* 根据状态决定显示登录界面还是帮助界面 */
    if (state->showHelp) {
        /* 绘制帮助界面 */
        settextcolor(params->WHITE_TEXT);
        
        /* 帮助界面标题 */
        settextstyle(36, 0, _T("Arial"));
        outtextxy(params->screenWidth / 2 - textwidth(_T("Help")) / 2, params->y - 100, _T("Help"));
        
        /* 恢复正常文本大小 */
        settextstyle(24, 0, _T("Arial"));
        
        /* 帮助内容 */
        int helpX = params->x - 250 , helpY = params->y;
        outtextxy(helpX, helpY, _T("1. Use RMB to fire with AATower"));
        helpY += 40;
        outtextxy(helpX, helpY, _T("2. Use Q~T to upgrade if there's enough skillpoints"));
        helpY += 40;
        outtextxy(helpX, helpY, _T("3. Use ESC to save and quit(the game will not save if ESC is not pressed!!!)"));
        helpY += 40;
        outtextxy(helpX, helpY, _T("4. New users will be automatically registered on first login(that is the username is not used)"));
        helpY += 40;
        outtextxy(helpX, helpY, _T("5. Don't fire for too long or the AATower will be overheated"));
        helpY += 40;
        
        /* 提示按H返回 */
        helpY += 40;
        outtextxy(params->screenWidth / 2 - textwidth(_T("Press H key to return to login screen")) / 2, helpY, _T("Press H key to return to login screen"));
    } else {
        /* 绘制登录界面 */
        /* 用户名字段 */
        setlinecolor(state->inputActive && !state->activeField ? params->ACCENT : 
                    (state->hoverUsername ? params->HOVER_COLOR : params->WHITE_TEXT));
        rectangle(params->x, params->y, params->x + params->boxWidth, params->y + params->boxHeight);

        /* 密码字段 */
        setlinecolor(state->inputActive && state->activeField ? params->ACCENT : 
                    (state->hoverPassword ? params->HOVER_COLOR : params->WHITE_TEXT));
        rectangle(params->x, params->y + 80, params->x + params->boxWidth, params->y + 80 + params->boxHeight);

        /* 绘制标签和输入内容 */
        settextcolor(params->WHITE_TEXT);
        outtextxy(params->x - 120, params->y + 10, _T("Username:"));
        outtextxy(params->x - 120, params->y + 90, _T("Password:"));
        outtextxy(params->x + 20, params->y + 10, state->user);
        outtextxy(params->x + 20, params->y + 90, state->pass);

        /* 绘制光标 */
        if (clock() - state->cursorTimer > 500 && state->inputActive) {
            state->showCursor = !state->showCursor;
            state->cursorTimer = clock();
        }
        if (state->showCursor && state->inputActive) {
            int textWidth = state->activeField ? textwidth(state->pass) : textwidth(state->user);
            int baseY = state->activeField ? params->y + 90 : params->y + 10;
            line(params->x + 20 + textWidth, baseY + 5, params->x + 20 + textWidth, baseY + 35);
        }

        /* 登录按钮 */
        int btnX = params->x + (params->boxWidth - params->btnWidth) / 2;
        int btnY = params->y + 180;
        setlinecolor(state->btnHover ? params->ACCENT : params->WHITE_TEXT);
        roundrect(btnX, btnY, btnX + params->btnWidth, btnY + params->btnHeight, 10, 10);

        /* 按钮文本 */
        settextcolor(params->WHITE_TEXT);
        int textWidth = textwidth(_T("LOGIN"));
        outtextxy(btnX + (params->btnWidth - textWidth) / 2, btnY + 15, _T("LOGIN"));
        
        /* 帮助提示 */
        settextcolor(params->HOVER_COLOR);
        outtextxy(params->screenWidth / 2 - textwidth(_T("Press H key for help")) / 2, params->y + 350, _T("Press H key for help"));

            /* 状态消息 */
        int msgY = btnY + params->btnHeight + 30;
        const TCHAR* msg = NULL;

        if (state->showError) {
            settextcolor(params->ERROR_RED);
            msg = _T("Invalid credentials!");
            outtextxy(params->screenWidth / 2 - textwidth(msg) / 2, msgY, msg);
        } else if (state->showSuccess) {
            settextcolor(params->SUCCESS_GREEN);
            msg = _T("Welcome!");
            outtextxy(params->screenWidth / 2 - textwidth(msg) / 2, msgY, msg);
            /* 处理成功登录 */
            FILE* savesFile = _tfopen(_T("saves.txt"), _T("r"));
            if (savesFile) {
                TCHAR line[256];
                while (_fgetts(line, sizeof(line) / sizeof(TCHAR), savesFile)) {
                    TCHAR username[16], password[16];
                    _stscanf(line, _T("{[%[^,],%[^]]]}"), username, password);
                    if (_tcscmp(state->user, username) == 0) {
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
            FlushBatchDraw();
            Sleep(2000);
            int msgX = params->screenWidth / 2 - textwidth(_T("Starting game...")) / 2;
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
            FlushBatchDraw();
        }
    }

    /* 绘制游戏元素 */
    drawAATower(params->screenWidth, params->screenHeight, state->angle);
    drawAABatteries(params->screenWidth, params->screenHeight);
    addAABatteriesProjectiles(params->screenWidth, params->screenHeight);
    simulateFiring();
    
    /* 更新探照灯角度 */
    state->angle = (state->angle + 1) % 360;
}

/* 主游戏循环函数 */
bool gameLoop(UIState* state, const UIParams* params) {
    handleMouseInput(state, params);
    handleKeyboardInput(state);
    drawUI(state, params);
    
    FlushBatchDraw();
    Sleep(10);
    
    return !state->showSuccess;
}

int main() {
    /* 初始化图形窗口和UI参数 */
    UIParams params = initializeUI();
    initgraph(params.screenWidth, params.screenHeight, SHOWCONSOLE);
    BeginBatchDraw();

    /* 初始化UI状态 */
    UIState state = initializeUIState();

    /* 设置图形设置 */
    setbkcolor(params.DARK_BG);
    cleardevice();
    settextstyle(24, 0, _T("Arial"));

    /* 主游戏循环 */
    while (gameLoop(&state, &params));

    EndBatchDraw();
    closegraph();
    return 0;
}
