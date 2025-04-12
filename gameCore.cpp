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

queue<Projectile> projectiles;
clock_t lastFiringTimeBattery1 = 0;
clock_t lastFiringTimeBattery2 = 0;

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

// 鼠标和防空塔相关变量
int mouseX = 0, mouseY = 0;
bool isMouseLeftDown = false;
clock_t lastFiringTimeTower = 0;
float towerHeatLevel = 0.0f;  // 防空塔过热度 (0.0 - 100.0)
bool isOverheated = false;     // 是否处于过热状态
const float MAX_HEAT_LEVEL = 100.0f;
const float HEAT_INCREASE_RATE = 5.0f;  // 每次射击增加的过热度
const float HEAT_DECREASE_RATE = 1.0f;  // 每帧冷却的过热度
const int TOWER_FIRE_COOLDOWN = 200;    // 射击冷却时间(毫秒)

// 升级界面相关变量
bool showUpgradeScreen = false;  // 是否显示升级界面

// 锁定飞机相关变量
bool isTargetLocked = false;  // 是否锁定了目标飞机
struct AircraftNode* lockedTarget = NULL;  // 被锁定的飞机指针
float currentSpotlightAngle1 = 0.0f;  // 当前探照灯1的角度
float currentSpotlightAngle2 = 0.0f;  // 当前探照灯2的角度
const float SPOTLIGHT_ROTATION_SPEED = 3.0f;  // 探照灯旋转速度(度/帧)

// 全局游戏数据
char global_username[21] = {0};
char global_password[21] = {0};
int score = 0;
int difficultyModifier = 0;
int skillPoints = 0;
// 防空塔升级
int AATowerUpgrade1 = 0;  // 每点使达到过热的时间+50%
int AATowerUpgrade2 = 0;  // 每点使防空塔射击间隔-20%
// 防空阵地升级
int AABatteriesUpgrade1 = 0;  // 每点使防空阵地连射最小值+1，随机修正部分+2
// 炮弹升级
int shellUpgrade1 = 0;  // 每点使炮弹伤害+50%
int shellUpgrade2 = 0;  // 每点使所有炮弹飞行速度+50%

// 在全局游戏数据部分添加以下内容
enum AircraftType { LIGHT, MEDIUM, HEAVY, SUPER_HEAVY };

struct Aircraft {
    AircraftType type;
    float x, y;
    float speed;
    float verticalSpeed; // 垂直速度
    int direction; // 1:从左向右飞 -1:从右向左飞
    int altitude;  // 0:低空 1:中空 2:高空
    int health;    // 飞机生命值
    bool isHit;    // 是否被击中
    clock_t hitTime; // 被击中的时间
    bool isDying;  // 死亡状态
    clock_t deathTime; // 死亡开始时间
    float explosionRadius; // 当前爆炸圆的半径
    int explosionStage; // 爆炸动画阶段
};

// 检查炮弹是否击中飞机
bool checkCollision(const Projectile& proj, const Aircraft& plane) {
    // 根据飞机类型定义碰撞箱大小
    int width, height;
    int leftOffset, rightOffset;
    
    switch(plane.type) {
        case LIGHT:
            // 根据绘制代码计算实际尺寸
            leftOffset = 60;  // -60到+25的范围
            rightOffset = 30;
            height = 30;      // 上下各15像素
            break;
        case MEDIUM:
            // -70到+40的范围
            leftOffset = 70;
            rightOffset = 40;
            height = 36;      // 上下各18像素
            break;
        case HEAVY:
            // -100到+20的范围
            leftOffset = 100;
            rightOffset = 20;
            height = 46;      // 上下各23像素
            break;
        case SUPER_HEAVY:
            // -150到+40的范围
            leftOffset = 150;
            rightOffset = 40;
            height = 60;      // 上下各30像素
            break;
    }
    
    // 根据飞机方向计算碰撞箱的边界
    float left = plane.x - leftOffset * plane.direction;
    float right = plane.x + rightOffset * plane.direction;
    if (plane.direction < 0) {
        // 如果飞机向左飞，交换左右边界
        float temp = left;
        left = right;
        right = temp;
    }
    float top = plane.y - height/2;
    float bottom = plane.y + height/2;
    
    // 检查炮弹是否在碰撞箱内
    return (proj.x >= left && proj.x <= right && 
            proj.y >= top && proj.y <= bottom);

}

// 处理飞机被击中的效果
void handleCollision(Aircraft& plane) {
    plane.isHit = true;
    plane.hitTime = clock();
    
    // 根据shellUpgrade1计算炮弹伤害
    int baseDamage = 10;
    int actualDamage = baseDamage * (100 + shellUpgrade1 * 50) / 100; // 每点shellUpgrade1增加50%伤害
    plane.health -= actualDamage;

    // 如果飞机血量降至0，初始化爆炸动画并增加分数
    if (plane.health <= 0 && !plane.isDying) {
        // 记录增加分数前的值
        int oldScore = score;
        
        // 根据飞机类型增加相应分数
        switch(plane.type) {
            case LIGHT:
                score += 40;
                break;
            case MEDIUM:
                score += 60;
                break;
            case HEAVY:
                score += 100;
                break;
            case SUPER_HEAVY:
                score += 150;
                break;
        }
        
        // 检查是否跨过1000分的倍数（技能点）
        int oldThousands = oldScore / 1000;
        int newThousands = score / 1000;
        if (newThousands > oldThousands) {
            skillPoints++;
        }
        
        // 检查是否跨过100分的倍数（难度修正）
        int oldHundreds = oldScore / 100;
        int newHundreds = score / 100;
        if (newHundreds > oldHundreds) {
            difficultyModifier++;
        }
        
        plane.isDying = true;
        plane.deathTime = clock();
        plane.explosionRadius = 0;
        plane.explosionStage = 0;
    }
}

// 定义飞机链表节点结构体
struct AircraftNode {
    Aircraft data;
    struct AircraftNode* next;
};

// 飞机链表头指针
struct AircraftNode* aircraftListHead = NULL;
clock_t lastAircraftGenTime = 0;

// 添加飞机到链表
void addAircraftToList(Aircraft newAircraft) {
    // 创建新节点
    struct AircraftNode* newNode = (struct AircraftNode*)malloc(sizeof(struct AircraftNode));
    if (newNode == NULL) {
        // 内存分配失败
        return;
    }
    
    // 设置节点数据
    newNode->data = newAircraft;
    newNode->next = NULL;
    
    // 如果链表为空，新节点成为头节点
    if (aircraftListHead == NULL) {
        aircraftListHead = newNode;
        return;
    }
    
    // 否则，将新节点添加到链表末尾
    struct AircraftNode* current = aircraftListHead;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

// 从链表中删除节点
void removeAircraftNode(struct AircraftNode** prevPtr, struct AircraftNode* nodeToRemove) {
    // 如果是头节点
    if (*prevPtr == NULL) {
        aircraftListHead = nodeToRemove->next;
    } else {
        (*prevPtr)->next = nodeToRemove->next;
    }
    
    // 释放节点内存
    free(nodeToRemove);
}

// 清理整个飞机链表
void cleanupAircraftList() {
    struct AircraftNode* current = aircraftListHead;
    struct AircraftNode* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    aircraftListHead = NULL;
}

// 在游戏循环前添加飞机生成函数
void generateAircrafts() {
    if (score >= 10000) return;
    if (clock() - lastAircraftGenTime > rand() % 3000 + 2000) { // 2-5秒生成间隔
        Aircraft newPlane;
        newPlane.type = static_cast<AircraftType>(rand() % 4);
        newPlane.direction = (rand() % 2) ? 1 : -1;
        newPlane.isHit = false;
        newPlane.verticalSpeed = 0;
        newPlane.isDying = false;
        newPlane.hitTime = 0;
        newPlane.explosionStage = 0;
        newPlane.explosionRadius = 0;
        
        // 根据机型设置初始生命值
        switch(newPlane.type) {
            case LIGHT:
                newPlane.health = 40 * (difficultyModifier+100) / 100;
                break;
            case MEDIUM:
                newPlane.health = 60 * (difficultyModifier+100) / 100;
                break;
            case HEAVY:
                newPlane.health = 100 * (difficultyModifier+100) / 100;
                break;
            case SUPER_HEAVY:
                newPlane.health = 150 * (difficultyModifier+100) / 100;
                break;
        }
        
        // 设置生成区域参数
        int genAreaTop = 0;
        int genAreaHeight = screenHeight * 2 / 3;
        int altitudeZone = 0;

        // 根据机型确定生成空域
        switch(newPlane.type) {
            case LIGHT:
                altitudeZone = rand() % 2; // 0-1
                break;
            case MEDIUM:
                altitudeZone = rand() % 3; // 0-2
                break; 
            case HEAVY:
                altitudeZone = 1 + rand() % 2; // 1-2
                break;
            case SUPER_HEAVY:
                altitudeZone = 2; // 固定高空
                break;
        }
        
        // 计算具体生成高度
        int zoneHeight = genAreaHeight / 3;
        // 将高度区域反向计算 (2 - altitudeZone)
        newPlane.y = genAreaTop + ((2 - altitudeZone) * zoneHeight) + rand() % (zoneHeight - 40);

        
        // 设置速度和初始位置
        switch(newPlane.type) {
            case LIGHT:
                newPlane.speed = 6.0f + (rand() % 200 - 100)/100.0f;
                break;
            case MEDIUM:
                newPlane.speed = 4.0f + (rand() % 200 - 100)/100.0f;
                break;
            case HEAVY:
                newPlane.speed = 2.0f + (rand() % 200 - 100)/100.0f;
                break;
            case SUPER_HEAVY:
                newPlane.speed = 1.0f + (rand() % 200 - 100)/100.0f;
                break;
        }
        
        newPlane.x = (newPlane.direction == 1) ? -100 : screenWidth + 100;
        addAircraftToList(newPlane);
        lastAircraftGenTime = clock();
    }
}

// 添加飞机更新函数
void updateAircrafts() {
    struct AircraftNode* current = aircraftListHead;
    struct AircraftNode* prev = NULL;
    
    while (current != NULL) {
        // 保存下一个节点的指针，因为当前节点可能会被删除
        struct AircraftNode* next = current->next;
        
        // 更新飞机位置
        current->data.x += current->data.speed * current->data.direction;
        
        // 检查飞机血量是否低于一半，根据机型判断初始血量
        int initialHealth;
        switch(current->data.type) {
            case LIGHT: initialHealth = 40; break;
            case MEDIUM: initialHealth = 60; break;
            case HEAVY: initialHealth = 100; break;
            case SUPER_HEAVY: initialHealth = 150; break;
        }
        
        // 如果血量低于一半，增加垂直速度以模拟下坠
        if (current->data.health < initialHealth / 2) {
            if (current->data.verticalSpeed == 0) {
                // 根据飞机的初始速度设置初始下落速度
                current->data.verticalSpeed = 0.1f * (current->data.speed / 6.0f); // 以最快的轻型飞机速度(6.0)为基准
            }
            // 垂直加速度也与飞机速度成正比
            current->data.verticalSpeed += 0.05f * (current->data.speed / 6.0f);
            current->data.y += current->data.verticalSpeed;
            
            // 检查飞机是否触底，如果触底则立刻将血量清零
            if (current->data.y >= screenHeight - 50 && !current->data.isDying) {
                current->data.health = 0;
                handleCollision(current->data);
            }
        }
        
        // 重置击中状态
        if (current->data.isHit && clock() - current->data.hitTime >= 100) {
            current->data.isHit = false;
        }

        // 更新爆炸动画
        if (current->data.isDying) {
            float maxRadius = 6000.0f;
            
            float timeSinceDeath = (clock() - current->data.deathTime) / 1000.0f;
            
            // 更新爆炸圆的半径
            if (timeSinceDeath < 0.5f) {
                current->data.explosionRadius = maxRadius * timeSinceDeath;
                setfillcolor(RGB(255, 0, 0));
                circle(current->data.x, current->data.y, current->data.explosionRadius);
            } else {
                // 爆炸动画结束，删除节点
                removeAircraftNode(&prev, current);
                current = next;
                continue;
            }
        }
        
        // 检查是否需要移除飞机
        if (!current->data.isDying && (
            (current->data.direction == 1 && current->data.x > screenWidth + 100) ||
            (current->data.direction == -1 && current->data.x < -100) ||
            current->data.y > screenHeight + 100)) {
            removeAircraftNode(&prev, current);
        } else {
            // 只有当节点没有被删除时，才更新prev指针
            prev = current;
        }
        
        // 移动到下一个节点
        current = next;
    }
}
// 改进后的飞机绘制函数
void drawAircrafts() {
    struct AircraftNode* current = aircraftListHead;
    
    while (current != NULL) {
        // 获取当前飞机数据的引用，使代码更简洁
        const Aircraft* plane = &(current->data);
        COLORREF mainColor, subColor, cockpitColor = RGB(255, 165, 0);
        double d = plane->direction;

        // 如果飞机被击中且在闪白时间内（100毫秒），使用白色绘制
        if (plane->isHit && clock() - plane->hitTime < 100) {
            mainColor = RGB(255, 255, 255);
            subColor = RGB(255, 255, 255);
            cockpitColor = RGB(255, 255, 255);
        } else {
            // 根据机型定制特征
            switch(plane->type) {
                case LIGHT:
                    mainColor = RGB(173, 216, 230);
                    subColor = RGB(32,93,93);
                    break;
                case MEDIUM:
                    mainColor = RGB(100,160,200);
                    subColor = RGB(0,153,136);
                    break;
                case HEAVY:
                    mainColor = RGB(0,102,178);
                    subColor = RGB(32,93,93);
                    break;
                case SUPER_HEAVY:
                    mainColor = RGB(25,25,112);
                    subColor = RGB(0,153,136);
                    break;
            }
        }

        // 如果飞机处于死亡状态，不绘制飞机模型
        if (!plane->isDying) {
            // 根据机型绘制飞机
            switch(plane->type) {
                case LIGHT:
                    //body
                    setfillcolor(mainColor*0.9);
                    fillrectangle(plane->x - d * 20, plane->y - 10, plane->x + d * 20, plane->y + 10);
                    fillrectangle(plane->x - d * 50, plane->y - 0, plane->x - d * 20, plane->y + 10);
                    fillrectangle(plane->x - d * 60, plane->y - 10, plane->x - d * 50, plane->y + 10);
                    //wing
                    setfillcolor(subColor);
                    fillrectangle(plane->x - d * 18, plane->y + 8, plane->x + d * 18, plane->y + 12);
                    fillrectangle(plane->x - d * 60, plane->y + 4, plane->x - d * 50, plane->y + 8);
                    //propeller
                    fillrectangle(plane->x + d * 20, plane->y - 3, plane->x + d * 25, plane->y + 3);
                    if(rand()%3==0){
                        fillrectangle(plane->x + d * 25, plane->y - 15, plane->x + d * 30, plane->y + 15);
                    }else if(rand()%3==1){
                        fillrectangle(plane->x + d * 25, plane->y - 10, plane->x + d * 30, plane->y + 10);
                    }else{
                        fillrectangle(plane->x + d * 25, plane->y - 3, plane->x + d * 30, plane->y + 3);
                    }
                    //cockpit
                    setfillcolor(cockpitColor);
                    fillrectangle(plane->x - d * 0, plane->y - 13, plane->x + d * 15, plane->y - 7);
                    break;

                case MEDIUM:
                    //body
                    setfillcolor(mainColor);
                    fillrectangle(plane->x - d * 20, plane->y - 15, plane->x + d * 20, plane->y + 15);
                    fillrectangle(plane->x - d * 30, plane->y - 10, plane->x - d * 20, plane->y + 10);
                    fillrectangle(plane->x - d * 60, plane->y - 0, plane->x - d * 30, plane->y + 10);
                    fillrectangle(plane->x - d * 70, plane->y - 15, plane->x - d * 60, plane->y + 10);
                    fillrectangle(plane->x + d * 20, plane->y - 5, plane->x + d * 30, plane->y + 15);
                    //wing
                    setfillcolor(subColor);
                    fillrectangle(plane->x - d * 18, plane->y + 12, plane->x + d * 18, plane->y + 18);
                    fillrectangle(plane->x - d * 70, plane->y + 4, plane->x - d * 50, plane->y + 8);
                    //propeller
                    fillrectangle(plane->x + d * 30, plane->y + 2, plane->x + d * 35, plane->y + 8);
                    if(rand()%3==0){
                        fillrectangle(plane->x + d * 35, plane->y - 15, plane->x + d * 40, plane->y + 25);
                    }else if(rand()%3==1){
                        fillrectangle(plane->x + d * 35, plane->y - 5, plane->x + d * 40, plane->y + 15);
                    }else{
                        fillrectangle(plane->x + d * 35, plane->y + 2, plane->x + d * 40, plane->y + 8);
                    }
                    //cockpit
                    setfillcolor(cockpitColor);
                    fillrectangle(plane->x + d * 1, plane->y - 18, plane->x + d * 15, plane->y - 12);
                    fillrectangle(plane->x - d * 15, plane->y - 18, plane->x - d * 1, plane->y - 12);
                    break;

                case HEAVY:
                    //body
                    setfillcolor(mainColor);
                    fillrectangle(plane->x - d * 40, plane->y - 15, plane->x + d * 30, plane->y + 15);
                    fillrectangle(plane->x - d * 80, plane->y - 15, plane->x - d * 40, plane->y + 0);
                    fillrectangle(plane->x - d * 100, plane->y - 20, plane->x - d * 80, plane->y + 5);
                    //wing
                    setfillcolor(subColor);
                    fillrectangle(plane->x - d * 30, plane->y - 3, plane->x + d * 10, plane->y + 3);
                    fillrectangle(plane->x - d * 10, plane->y + 3, plane->x + d * 10, plane->y + 13);
                    //propeller
                    fillrectangle(plane->x + d * 10, plane->y + 5, plane->x + d * 15, plane->y + 11);
                    if(rand()%3==0){
                        fillrectangle(plane->x + d * 15, plane->y - 7, plane->x + d * 20, plane->y + 23);
                    }else if(rand()%3==1){
                        fillrectangle(plane->x + d * 15, plane->y - 0, plane->x + d * 20, plane->y + 16);
                    }else{
                        fillrectangle(plane->x + d * 15, plane->y + 5, plane->x + d * 20, plane->y + 11);
                    }
                    //cockpit
                    setfillcolor(cockpitColor);
                    fillrectangle(plane->x - d * 5, plane->y - 18, plane->x + d * 20, plane->y - 12);
                    break;

                case SUPER_HEAVY:
                    //body
                    setfillcolor(mainColor);
                    fillrectangle(plane->x - d * 50, plane->y - 20, plane->x + d * 50, plane->y + 20);
                    fillrectangle(plane->x - d * 90, plane->y - 20, plane->x - d * 50, plane->y + 10);
                    fillrectangle(plane->x - d * 130, plane->y - 20, plane->x - d * 90, plane->y + 0);
                    fillrectangle(plane->x - d * 150, plane->y - 30, plane->x - d * 130, plane->y + 10);
                    //wing
                    setfillcolor(subColor);
                    fillrectangle(plane->x - d * 30, plane->y - 3, plane->x + d * 30, plane->y + 3);
                    fillrectangle(plane->x + d * 10, plane->y + 3, plane->x + d * 30, plane->y + 13);
                    fillrectangle(plane->x - d * 5, plane->y + 3, plane->x + d * 15, plane->y + 13);
                    //propeller
                    fillrectangle(plane->x + d * 30, plane->y + 5, plane->x + d * 35, plane->y + 11);
                    fillrectangle(plane->x + d * 15, plane->y + 5, plane->x + d * 20, plane->y + 11);
                    if(rand()%3==0){
                        fillrectangle(plane->x + d * 35, plane->y - 7, plane->x + d * 40, plane->y + 23);
                    }else if(rand()%3==1){
                        fillrectangle(plane->x + d * 35, plane->y - 0, plane->x + d * 40, plane->y + 16);
                    }else{
                        fillrectangle(plane->x + d * 35, plane->y + 5, plane->x + d * 40, plane->y + 11);
                    }
                    if(rand()%3==0){
                        fillrectangle(plane->x + d * 20, plane->y - 7, plane->x + d * 25, plane->y + 23);
                    }else if(rand()%3==1){
                        fillrectangle(plane->x + d * 20, plane->y - 0, plane->x + d * 25, plane->y + 16);
                    }else{
                        fillrectangle(plane->x + d * 20, plane->y + 5, plane->x + d * 25, plane->y + 11);
                    }
                    //cockpit
                    setfillcolor(cockpitColor);
                    fillrectangle(plane->x + d * 16, plane->y - 23, plane->x + d * 40, plane->y - 17);
                    fillrectangle(plane->x - d * 10, plane->y - 23, plane->x + d * 14, plane->y - 17);
                    break;
            }
        } else {
            // 如果飞机处于死亡状态，绘制爆炸动画
            float maxRadius;
            switch(plane->type) {
                case LIGHT: maxRadius = 100.0f; break;
                case MEDIUM: maxRadius = 200.0f; break;
                case HEAVY: maxRadius = 400.0f; break;
                case SUPER_HEAVY: maxRadius = 800.0f; break;
            }
            
            float timeSinceDeath = (clock() - plane->deathTime) / 1000.0f;
            if (timeSinceDeath < 0.25f) {
                setfillcolor(RGB(255, 0, 0));
                solidcircle(plane->x, plane->y, maxRadius * timeSinceDeath);
            }
        }
        
        // 移动到下一个节点
        current = current->next;
    }
}

// 防空炮塔绘制
// Function to add new projectiles from anti-aircraft batteries
void addAABatteriesProjectiles(int screenWidth, int screenHeight) {
    int battery1X = screenWidth / 4;
    int battery2X = screenWidth * 3 / 4;
    int batteryY = screenHeight - 30;

    // Check if battery 1 is ready to fire
    if (score < 10000 && clock() - lastFiringTimeBattery1 > rand() % 3000 + 1000) { // 胜利后停止射击
        // Calculate firing angle for battery 1
        float angle1;
        
        // 如果有飞机被锁定，瞄准该飞机射击
        if (isTargetLocked && lockedTarget != NULL) {
            // 计算从防空阵地1到锁定飞机的角度
            float dx = lockedTarget->data.x - battery1X;
            float dy = lockedTarget->data.y - batteryY;
            float targetAngleRad = atan2(-dy, dx);
            angle1 = targetAngleRad * 180.0f / 3.14159265f;
            
            // 确保角度在合理范围内（向上射击）
            if (angle1 < 0) angle1 += 360.0f;
            // 限制角度在30-150度范围内
            angle1 = max(30.0f, min(150.0f, angle1));
        } else {
            // 未锁定目标时使用随机角度
            angle1 = 45.0f + static_cast<float>(rand() % 90); // 角度在45到135度之间
        }

        // Create projectiles for battery 1
        // 根据AABatteriesUpgrade1增加连射最小值和随机修正
        int minShots = 2 + AABatteriesUpgrade1; // 最小值+1每点
        int randomModifier = 8 + AABatteriesUpgrade1 * 2; // 随机修正+2每点
        for (int i = 0; i < (rand() % randomModifier) + minShots; ++i) {
            Projectile p1;
            p1.x = battery1X + (rand() % 20 - 10);
            p1.y = batteryY - 20; // Start position above battery
            p1.life = 2000; // Projectile lifetime
            p1.angle = angle1; // Store firing angle
            p1.fireTime = clock() + i * 100; // Staggered firing timing
            projectiles.push(p1);
        }

        lastFiringTimeBattery1 = clock(); // Update last firing time
    }

    // Check if battery 2 is ready to fire
    if (score < 10000 && clock() - lastFiringTimeBattery2 > rand() % 3000 + 1000) { // 胜利后停止射击
        // Calculate firing angle for battery 2
        float angle2;
        
        // 如果有飞机被锁定，瞄准该飞机射击
        if (isTargetLocked && lockedTarget != NULL) {
            // 计算从防空阵地2到锁定飞机的角度
            float dx = lockedTarget->data.x - battery2X;
            float dy = lockedTarget->data.y - batteryY;
            float targetAngleRad = atan2(-dy, dx);
            angle2 = targetAngleRad * 180.0f / 3.14159265f;
            
            // 确保角度在合理范围内（向上射击）
            if (angle2 < 0) angle2 += 360.0f;
            // 限制角度在30-150度范围内
            angle2 = max(30.0f, min(150.0f, angle2));
        } else {
            // 未锁定目标时使用随机角度
            angle2 = 45.0f + static_cast<float>(rand() % 90); // 角度在45到135度之间
        }

        // Create projectiles for battery 2
        for (int i = 0; i < (rand()%8)+2 ; ++i) {
            Projectile p2;
            p2.x = battery2X + (rand() % 20 - 10);
            p2.y = batteryY - 20; // Start position above battery
            p2.life = 2000; // Projectile lifetime
            p2.angle = angle2; // Store firing angle
            p2.fireTime = clock() + i * 100; // Staggered firing timing
            projectiles.push(p2);
        }

        lastFiringTimeBattery2 = clock(); // Update last firing time
    }
}



// 添加防空塔射击函数
void addAATowerProjectiles(int screenWidth, int screenHeight) {
    int towerX = screenWidth / 2;
    int towerY = screenHeight - 30 - 80; // 塔顶位置 (baseY - towerHeight)
    
    // 检查鼠标是否按下且防空塔未处于过热状态
    // 根据AATowerUpgrade1调整过热阈值，每点使达到过热的时间+50%
    float overHeatThreshold = MAX_HEAT_LEVEL * (0.9f + AATowerUpgrade1 * 0.5f);
    // 确保阈值不超过最大热量的3倍
    overHeatThreshold = min(overHeatThreshold, MAX_HEAT_LEVEL * 3.0f);
    
    if (!isOverheated && towerHeatLevel >= overHeatThreshold) {
        isOverheated = true;
    } else if (isOverheated && towerHeatLevel <= 0) {
        isOverheated = false;
    }
    
    if (isMouseLeftDown && !isOverheated) {
        // 检查冷却时间，根据AATowerUpgrade2减少射击间隔，每点-20%
        int adjustedCooldown = TOWER_FIRE_COOLDOWN * (100 - AATowerUpgrade2 * 20) / 100;
        // 确保冷却时间不低于最小值
        adjustedCooldown = max(adjustedCooldown, 50);
        
        if (clock() - lastFiringTimeTower > adjustedCooldown) {
            // 计算射击角度 (从塔顶到鼠标位置)
            float dx = mouseX - towerX;
            float dy = mouseY - towerY;
            float angle = atan2(-dy, dx) * 180 / 3.14159265;
            
            // 创建炮弹
            Projectile p;
            p.x = towerX;
            p.y = towerY;
            p.life = 2000; // 炮弹生命周期
            p.angle = angle; // 存储射击角度
            p.fireTime = clock(); // 立即发射
            projectiles.push(p);
            
            // 更新上次射击时间
            lastFiringTimeTower = clock();
            
            // 增加过热度
            towerHeatLevel += HEAT_INCREASE_RATE;
        }
    } else {
        // 如果没有射击，降低过热度
        // 在过热状态下冷却速度降为四分之一
        float currentCoolRate = isOverheated ? (HEAT_DECREASE_RATE / 4.0f) : HEAT_DECREASE_RATE;
        towerHeatLevel -= currentCoolRate;
        if (towerHeatLevel < 0) {
            towerHeatLevel = 0;
        }
    }
}

// Function to simulate firing from anti-aircraft batteries
void simulateFiring() {
    // Update and render projectiles
    double baseSpeed = 10.0; // 基础炮弹速度
    double pSpeed = baseSpeed * (100 + shellUpgrade2 * 50) / 100; // 每点shellUpgrade2增加50%速度
    clock_t currentTime = clock(); // Get the current time once
    
    // 使用临时队列来保存遍历过程中的元素
    queue<Projectile> tempQueue;
    int queueSize = projectiles.size();
    
    // 遍历队列中的所有炮弹
    for (int i = 0; i < queueSize; i++) {
        // 获取队首元素
        Projectile currentProj = projectiles.front();
        projectiles.pop(); // 从原队列中移除
        
        bool projectileHit = false;
        
        // 检查炮弹是否击中任何飞机 - 使用链表遍历
        struct AircraftNode* current = aircraftListHead;
        while (current != NULL) {
            if (checkCollision(currentProj, current->data)) {
                handleCollision(current->data);
                projectileHit = true;
                // 显示爆炸效果
                setfillcolor(RGB(255, 255, 0));
                solidcircle(currentProj.x, currentProj.y, 30);
                break;
            }
            current = current->next;
        }
        
        if (currentTime >= currentProj.fireTime && currentProj.life > 0) {  // Check if it's time to fire the projectile
            if (projectileHit) {
                // 被击中的炮弹不放回队列
                continue;
            }
            setfillcolor(RGB(255, 0, 0)); // 设置炮弹颜色
            solidcircle(currentProj.x, currentProj.y, 3); // Draw projectile
            currentProj.y -= pSpeed * sin(currentProj.angle * 3.14159265 / 180); // Move the projectile based on its angle
            currentProj.x += pSpeed * cos(currentProj.angle * 3.14159265 / 180); // Move to the direction of the angle
            currentProj.life--; // Decrease life
            
            // 如果炮弹仍然有生命值，放回临时队列
            if (currentProj.life > 0) {
                tempQueue.push(currentProj);
            }
        } else if (currentProj.life <= 0) {
            // 生命值为0的炮弹不放回队列
        } else {
            // 未到发射时间的炮弹放回队列
            tempQueue.push(currentProj);
        }
    }
    
    // 将临时队列中的元素放回原队列
    projectiles = tempQueue;
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

    // 探照灯起始位置
    int light1X1 = baseX - 50;
    int light1Y1 = baseY - towerHeight;
    int light2X1 = baseX + 50;
    int light2Y1 = baseY - towerHeight;
    
    // 计算探照灯角度和终点
    float targetAngle1, targetAngle2;
    int light1X2, light1Y2, light2X2, light2Y2;
    
    if (isTargetLocked && lockedTarget != NULL) {
        // 计算从塔到锁定飞机的角度
        float dx = lockedTarget->data.x - baseX;
        float dy = lockedTarget->data.y - (baseY - towerHeight);
        float targetAngleRad = atan2(-dy, dx);
        targetAngle1 = targetAngleRad * 180.0f / 3.14159265f;
        targetAngle2 = targetAngle1;
        
        // 平滑旋转探照灯1
        if (currentSpotlightAngle1 != targetAngle1) {
            float angleDiff = targetAngle1 - currentSpotlightAngle1;
            // 处理角度跨越360度的情况
            if (angleDiff > 180) angleDiff -= 360;
            if (angleDiff < -180) angleDiff += 360;
            
            // 限制旋转速度
            if (angleDiff > SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle1 += SPOTLIGHT_ROTATION_SPEED;
            } else if (angleDiff < -SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle1 -= SPOTLIGHT_ROTATION_SPEED;
            } else {
                currentSpotlightAngle1 = targetAngle1;
            }
        }
        
        // 平滑旋转探照灯2
        if (currentSpotlightAngle2 != targetAngle2) {
            float angleDiff = targetAngle2 - currentSpotlightAngle2;
            // 处理角度跨越360度的情况
            if (angleDiff > 180) angleDiff -= 360;
            if (angleDiff < -180) angleDiff += 360;
            
            // 限制旋转速度
            if (angleDiff > SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle2 += SPOTLIGHT_ROTATION_SPEED;
            } else if (angleDiff < -SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle2 -= SPOTLIGHT_ROTATION_SPEED;
            } else {
                currentSpotlightAngle2 = targetAngle2;
            }
        }
    } else {
        // 未锁定目标时，探照灯回到扫描状态
        targetAngle1 = (abs(45 - (0.25 * ((angle) % 360))) + 90);
        targetAngle2 = (abs(45 - (0.25 * ((angle + 36) % 360))) + 45);
        
        // 平滑旋转探照灯1
        if (currentSpotlightAngle1 != targetAngle1) {
            float angleDiff = targetAngle1 - currentSpotlightAngle1;
            // 处理角度跨越360度的情况
            if (angleDiff > 180) angleDiff -= 360;
            if (angleDiff < -180) angleDiff += 360;
            
            // 限制旋转速度
            if (angleDiff > SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle1 += SPOTLIGHT_ROTATION_SPEED;
            } else if (angleDiff < -SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle1 -= SPOTLIGHT_ROTATION_SPEED;
            } else {
                currentSpotlightAngle1 = targetAngle1;
            }
        }
        
        // 平滑旋转探照灯2
        if (currentSpotlightAngle2 != targetAngle2) {
            float angleDiff = targetAngle2 - currentSpotlightAngle2;
            // 处理角度跨越360度的情况
            if (angleDiff > 180) angleDiff -= 360;
            if (angleDiff < -180) angleDiff += 360;
            
            // 限制旋转速度
            if (angleDiff > SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle2 += SPOTLIGHT_ROTATION_SPEED;
            } else if (angleDiff < -SPOTLIGHT_ROTATION_SPEED) {
                currentSpotlightAngle2 -= SPOTLIGHT_ROTATION_SPEED;
            } else {
                currentSpotlightAngle2 = targetAngle2;
            }
        }
    }
    
    // 确保角度在0-360范围内
    currentSpotlightAngle1 = fmod(currentSpotlightAngle1 + 360, 360);
    currentSpotlightAngle2 = fmod(currentSpotlightAngle2 + 360, 360);
    
    // 计算探照灯终点
    light1X2 = light1X1 + 2000 * cos(currentSpotlightAngle1 * 3.14159265 / 180);
    light1Y2 = light1Y1 - 2000 * sin(currentSpotlightAngle1 * 3.14159265 / 180);
    light2X2 = light2X1 + 2000 * cos(currentSpotlightAngle2 * 3.14159265 / 180);
    light2Y2 = light2Y1 - 2000 * sin(currentSpotlightAngle2 * 3.14159265 / 180);

    // 根据是否锁定目标设置探照灯颜色
    if (isTargetLocked && lockedTarget != NULL) {
        setlinecolor(RGB(255, 0, 0)); // 锁定目标时为红色
    } else {
        setlinecolor(RGB(255, 255, 0)); // 正常扫描时为黄色
    }
    
    // 绘制探照灯
    line(light1X1, light1Y1, light1X2, light1Y2);
    line(light2X1, light2Y1, light2X2, light2Y2);
    
    // 绘制过热度指示器（纵向，在防空塔内部）
    int heatBarWidth = 20;
    int heatBarHeight = towerHeight - 20; // 留出一些边距
    int heatBarX = baseX - heatBarWidth/2;
    int heatBarY = baseY - 10; // 距离底部留出一些空间
    
    // 绘制过热度条的边框
    setlinecolor(RGB(128, 128, 128));
    rectangle(heatBarX, heatBarY, heatBarX + heatBarWidth, heatBarY - heatBarHeight);
    
    // 计算过热阈值（与addAATowerProjectiles函数中相同的计算方式）
    float overHeatThreshold = MAX_HEAT_LEVEL * (0.9f + AATowerUpgrade1 * 0.5f);
    overHeatThreshold = min(overHeatThreshold, MAX_HEAT_LEVEL * 3.0f);
    
    // 计算当前过热度对应的高度（使用百分比而非绝对值）
    float heatPercentage = towerHeatLevel / overHeatThreshold;
    // 限制百分比最大为100%
    heatPercentage = min(heatPercentage, 1.0f);
    int currentHeatHeight = (int)(heatPercentage * heatBarHeight);
    
    // 根据过热状态设置颜色
    if (isOverheated) {
        setfillcolor(RGB(255, 0, 0)); // 过热状态显示纯红色，直到过热状态解除
    } else {
        // 根据过热度百分比从黄色(0%)渐变到红色(100%)
        // 黄色是RGB(255,255,0)，红色是RGB(255,0,0)
        // 所以只需要根据百分比调整绿色通道的值
        int greenValue = 255 - (int)(255 * heatPercentage);
        setfillcolor(RGB(255, greenValue, 0));
    }
    
    // 从下向上填充过热度条
    if (currentHeatHeight > 0) {
        fillrectangle(heatBarX + 2, heatBarY - 2, 
                     heatBarX + heatBarWidth - 2, 
                     heatBarY - currentHeatHeight + 2);
    }

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
// 在屏幕左上角绘制游戏数据
void drawGameStats() {
    // 设置文本属性
    settextcolor(RGB(255, 255, 255));  // 白色文本
    settextstyle(20, 0, _T("Consolas"));
    setbkmode(TRANSPARENT);

    // 创建三行文本缓冲区
    TCHAR line1[32], line2[32], line3[32];
    
    // 分别格式化三行文本
    _stprintf(line1, _T("Score: %10d"), score);
    _stprintf(line2, _T("Difficulty: %5d"), difficultyModifier);
    _stprintf(line3, _T("Skill Points: %3d"), skillPoints);

    // 逐行绘制（行间距25像素）
    outtextxy(10, 10, line1);    // 第一行
    outtextxy(10, 35, line2);    // 第二行（10+25）
    outtextxy(10, 60, line3);    // 第三行（35+25）
}

// 绘制升级界面
void drawUpgradeScreen() {
    // 设置文本属性
    settextcolor(RGB(255, 255, 255));  // 白色文本
    setbkmode(TRANSPARENT);
    
    // 绘制标题
    settextstyle(36, 0, _T("Arial"));
    TCHAR title[32] = _T("Upgrades");
    int titleWidth = textwidth(title);
    outtextxy((screenWidth - titleWidth) / 2, 50, title);
    
    // 显示剩余技能点
    settextstyle(24, 0, _T("Arial"));
    TCHAR skillPointsText[64];
    _stprintf(skillPointsText, _T("Available Skill Points: %d"), skillPoints);
    int skillPointsWidth = textwidth(skillPointsText);
    outtextxy((screenWidth - skillPointsWidth) / 2, 100, skillPointsText);
    
    // 设置升级项目文本样式
    settextstyle(20, 0, _T("Consolas"));
    
    // 绘制升级选项
    int startY = 150;
    int lineHeight = 40;
    
    // 升级选项1: AATowerUpgrade1
    TCHAR upgrade1[128];
    _stprintf(upgrade1, _T("AATowerUpgrade1 (Q): Increase overheat time by 50%% per level. Current level: %d"), AATowerUpgrade1);
    outtextxy(100, startY, upgrade1);
    
    // 升级选项2: AATowerUpgrade2
    TCHAR upgrade2[128];
    _stprintf(upgrade2, _T("AATowerUpgrade2 (W): Decrease firing interval by 20%% per level. Current level: %d"), AATowerUpgrade2);
    outtextxy(100, startY + lineHeight, upgrade2);
    
    // 升级选项3: AABatteriesUpgrade1
    TCHAR upgrade3[128];
    _stprintf(upgrade3, _T("AABatteriesUpgrade1 (E): Increase AA batteries min shots by 1 and random by 2. Current level: %d"), AABatteriesUpgrade1);
    outtextxy(100, startY + lineHeight * 2, upgrade3);
    
    // 升级选项4: shellUpgrade1
    TCHAR upgrade4[128];
    _stprintf(upgrade4, _T("shellUpgrade1 (R): Increase shell damage by 50%% per level. Current level: %d"), shellUpgrade1);
    outtextxy(100, startY + lineHeight * 3, upgrade4);
    
    // 升级选项5: shellUpgrade2
    TCHAR upgrade5[128];
    _stprintf(upgrade5, _T("shellUpgrade2 (T): Increase all shell speed by 50%% per level. Current level: %d"), shellUpgrade2);
    outtextxy(100, startY + lineHeight * 4, upgrade5);
    
    // 返回游戏提示
    settextstyle(24, 0, _T("Arial"));
    TCHAR returnText[64] = _T("Press U to return to game");
    int returnWidth = textwidth(returnText);
    outtextxy((screenWidth - returnWidth) / 2, startY + lineHeight * 6, returnText);
}

void setUp(){
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
                &AATowerUpgrade1, &AATowerUpgrade2,
                &AABatteriesUpgrade1, &shellUpgrade1, &shellUpgrade2);
            
            // 保存到全局变量
            strcpy(global_username, username_buf);
            strcpy(global_password, password_buf);
        }
        activeFile.close();
    }
}

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
        << AATowerUpgrade1 << "," << AATowerUpgrade2 << "," 
        << AABatteriesUpgrade1 << "," << shellUpgrade1 << "," 
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

// 游戏主循环函数
void gameLoop() {
    int angle = 0;
    bool running = true;
    
    while (running) {
        // 处理输入
        if (_kbhit()) {
            int key = _getch();
            switch (key) {
                case 27: // ESC键
                    running = false;
                    break;
                case 'u': // U键切换升级界面
                case 'U':
                    showUpgradeScreen = !showUpgradeScreen;
                    break;
                    
                // 只有在升级界面中Q~T键才能用于升级
                case 'q':
                case 'Q':
                    if(showUpgradeScreen && skillPoints > 0) {
                        AATowerUpgrade1++;
                        skillPoints--;
                    }
                    break;
                case 'w':
                case 'W':
                    if(showUpgradeScreen && skillPoints > 0) {
                        AATowerUpgrade2++;
                        skillPoints--;
                    }
                    break;
                case 'e':
                case 'E':
                    if(showUpgradeScreen && skillPoints > 0) {
                        AABatteriesUpgrade1++;
                        skillPoints--;
                    }
                    break;
                case 'r':
                case 'R':
                    if(showUpgradeScreen && skillPoints > 0) {
                        shellUpgrade1++;
                        skillPoints--;
                    }
                    break;
                case 't':
                case 'T':
                    if(showUpgradeScreen && skillPoints > 0) {
                        shellUpgrade2++;
                        skillPoints--;
                    }
                    break;
            }
        }

        // 处理鼠标事件
        if (MouseHit()) {
            MOUSEMSG m = GetMouseMsg();
            mouseX = m.x;
            mouseY = m.y;
            
            // 检测鼠标左键按下和释放
            if (m.uMsg == WM_LBUTTONDOWN) {
                isMouseLeftDown = true;
            } else if (m.uMsg == WM_LBUTTONUP) {
                isMouseLeftDown = false;
            } else if (m.uMsg == WM_RBUTTONDOWN) {
                // 右键点击 - 寻找最近的飞机并锁定
                struct AircraftNode* current = aircraftListHead;
                struct AircraftNode* closestPlane = NULL;
                float minDistance = FLT_MAX;
                
                // 遍历所有飞机，找出最近的一架
                while (current != NULL) {
                    // 只考虑未死亡的飞机
                    if (!current->data.isDying) {
                        float dx = current->data.x - mouseX;
                        float dy = current->data.y - mouseY;
                        float distance = sqrt(dx*dx + dy*dy);
                        
                        if (distance < minDistance) {
                            minDistance = distance;
                            closestPlane = current;
                        }
                    }
                    current = current->next;
                }
                
                // 如果找到了飞机，锁定它
                if (closestPlane != NULL) {
                    lockedTarget = closestPlane;
                    isTargetLocked = true;
                }
            }
        }
        
        // 检查锁定的飞机是否还存在或已被摧毁
        if (isTargetLocked) {
            bool targetFound = false;
            struct AircraftNode* current = aircraftListHead;
            
            while (current != NULL) {
                if (current == lockedTarget && !current->data.isDying) {
                    targetFound = true;
                    break;
                }
                current = current->next;
            }
            
            // 如果目标不存在或已被摧毁，解除锁定
            if (!targetFound) {
                isTargetLocked = false;
                lockedTarget = NULL;
            }
        }

        // 更新游戏状态
        cleardevice();
        
        // 只有在不显示升级界面时才更新游戏
        if (!showUpgradeScreen) {
            angle = (angle + 1) % 360;

            // 新增飞机系统调用
            generateAircrafts();
            updateAircrafts();
            
            // 处理防空塔射击
            addAATowerProjectiles(screenWidth, screenHeight);
            
            // 绘制游戏元素
            drawAircrafts();
            drawAATower(screenWidth, screenHeight, angle);
            drawAABatteries(screenWidth, screenHeight);
            addAABatteriesProjectiles(screenWidth, screenHeight);
            simulateFiring();
        }

        // 始终显示游戏统计信息
        drawGameStats();
        
        // 如果显示升级界面，则绘制升级界面
        if (showUpgradeScreen) {
            drawUpgradeScreen();
        }

        if (score >= 10000) {
            settextcolor(RGB(255, 215, 0));
            settextstyle(48, 0, _T("Arial"));
            outtextxy(screenWidth/2 - 300, screenHeight/2 - 50, _T("Enemy is retreating! Our city is saved!"));
            
            settextcolor(RGB(50, 205, 50));
            settextstyle(24, 0, _T("Arial"));
            outtextxy(screenWidth/2 - 150, 50, _T("Press ESC to save and exit"));
        }

        FlushBatchDraw();
        Sleep(10);
    }
}

int main() {
    // 初始化图形窗口
    initgraph(screenWidth, screenHeight, SHOWCONSOLE);
    BeginBatchDraw();

    // 初始化游戏数据
    setUp();

    // 运行游戏主循环
    gameLoop();

    // 保存游戏数据
    saveGameData(global_username, global_password);

    // 清理资源
    EndBatchDraw();
    closegraph();
    return 0;
}
