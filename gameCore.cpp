#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <bits/stdc++.h>
using namespace std;

// 弹丸数据结构
struct Projectile {
    int x;
    int y;
    int life;
    float angle;
    clock_t fireTime;
};

vector<Projectile> projectiles;
clock_t lastFiringTimeBattery1 = 0;
clock_t lastFiringTimeBattery2 = 0;

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

// 防空炮塔绘制
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

// 全局游戏数据
int score = 0;
int difficultyModifier = 0;
int skillPoints = 0;
int AATowerUpgrade1 = 0;
int AABatteriesUpgrade1 = 0;
int AABatteriesUpgrade2 = 0;
int shellUpgrade1 = 0;
int shellUpgrade2 = 0;

void saveGameData(const char* username, const char* password) {
    cleardevice();
    settextcolor(WHITE);
    settextstyle(36, 0, _T("Arial"));
    const TCHAR* msg = _T("Saving Data.");
    int msgX = (screenWidth - textwidth(msg)) / 2;
    int msgY = screenHeight / 2;
    
    outtextxy(msgX, msgY, msg);
    FlushBatchDraw();
    Sleep(500);
    
    msg = _T("Saving Data..");
    outtextxy(msgX, msgY, msg);
    FlushBatchDraw();
    Sleep(500);
    
    msg = _T("Saving Data...");
    outtextxy(msgX, msgY, msg);
    FlushBatchDraw();
    Sleep(500);

    // 构建保存数据
    ostringstream oss;
    oss << "{[" << username << "," << password << "],["
        << score << "," << difficultyModifier << "," << skillPoints << "],["
        << AATowerUpgrade1 << "," << AABatteriesUpgrade1 << "," 
        << AABatteriesUpgrade2 << "," << shellUpgrade1 << "," 
        << shellUpgrade2 << "]}";

    // 写入activeUser
    ofstream outActive("activeUser.txt");
    if (outActive) {
        outActive << oss.str() << '\n';
        outActive.close();
    }

    // 更新saves.txt
    vector<string> saves;
    ifstream inSaves("saves.txt");
    if (inSaves) {
        string line;
        while (getline(inSaves, line)) {
            char check_user[21] = {0};
            if (sscanf(line.c_str(), "{[%20[^,],", check_user) == 1) {
                if (string(check_user) == username) {
                    saves.push_back(oss.str());
                    continue;
                }
            }
            saves.push_back(line);
        }
        inSaves.close();
    }

    ofstream outSaves("saves.txt");
    if (outSaves) {
        for (const auto& entry : saves) {
            outSaves << entry << '\n';
        }
        outSaves.close();
    }
}

int main() {
    // 初始化图形窗口
    initgraph(screenWidth, screenHeight, SHOWCONSOLE);
    BeginBatchDraw();

    // 加载用户数据
    char username_buf[21] = {0};
    char password_buf[21] = {0};
    
    ifstream activeFile("activeUser.txt");
    if (activeFile) {
        string line;
        if (getline(activeFile, line)) {
            sscanf(line.c_str(), 
                "{[%20[^,],%20[^]]],[%d,%d,%d],[%d,%d,%d,%d,%d]}",
                username_buf, password_buf,
                &score, &difficultyModifier, &skillPoints,
                &AATowerUpgrade1, &AABatteriesUpgrade1,
                &AABatteriesUpgrade2, &shellUpgrade1, &shellUpgrade2);
        }
        activeFile.close();
    }

    // 游戏主循环
    int angle = 0;
    bool running = true;
    
    while (running) {
        // 处理输入
        if (_kbhit()) {
            int key = _getch();
            if (key == 27) { // ESC键
                running = false;
            }
        }

        // 更新游戏状态
        cleardevice();
        angle = (angle + 1) % 360;
        
        // 绘制游戏元素
        drawAATower(screenWidth, screenHeight, angle);
        drawAABatteries(screenWidth, screenHeight);
        addAABatteriesProjectiles(screenWidth, screenHeight);
        simulateFiring();

        // 示例游戏逻辑
        score += 1;
        
        FlushBatchDraw();
        Sleep(10);
    }

    // 保存游戏数据
    saveGameData(username_buf, password_buf);

    // 清理资源
    EndBatchDraw();
    closegraph();
    return 0;
}
