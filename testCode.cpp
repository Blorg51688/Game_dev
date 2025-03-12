#include <graphics.h>
#include <conio.h>
#include <iostream>

int main()
{
    // 初始化图形窗口
    initgraph(640, 480);
    setbkcolor(WHITE);
    cleardevice();
    
    // 启用控制台窗口用于调试输出
    HWND hwnd = GetForegroundWindow();
    ShowWindow(hwnd, SW_SHOW);
    
    // 输入状态跟踪
    int keyCode = -1;
    char keyChar = 0;
    int mouseX = 0, mouseY = 0;
    bool leftDown = false;
    bool rightDown = false;

    // 主循环
    while (true)
    {
        // 处理键盘输入
        if (_kbhit())
        {
            keyCode = _getch();
            keyChar = (keyCode >= 32 && keyCode <= 126) ? keyCode : 0;
            
            // 调试输出到控制台
            std::cout << "Key detected: " << keyCode 
                      << " (" << keyChar << ")\n";
            
            if (keyCode == 27) break; // ESC退出
        }

        // 处理鼠标输入
        MOUSEMSG m;
        while (MouseHit())
        {
            m = GetMouseMsg();
            mouseX = m.x;
            mouseY = m.y;
            
            switch(m.uMsg)
            {
            case WM_LBUTTONDOWN:
                leftDown = true;
                std::cout << "Left click at (" 
                          << mouseX << "," << mouseY << ")\n";
                break;
            case WM_LBUTTONUP:
                leftDown = false;
                break;
            case WM_RBUTTONDOWN:
                rightDown = true;
                std::cout << "Right click at (" 
                          << mouseX << "," << mouseY << ")\n";
                break;
            case WM_RBUTTONUP:
                rightDown = false;
                break;
            }
        }

        // 图形界面绘制
        cleardevice();
        
        // 绘制键盘状态
        settextcolor(BLACK);
        outtextxy(20, 20, "Keyboard Status:");
        char keyStr[50];
        sprintf(keyStr, "Code: %d  Char: %c", keyCode, keyChar);
        outtextxy(40, 50, keyStr);

        // 绘制鼠标状态
        outtextxy(20, 100, "Mouse Status:");
        char mouseStr[100];
        sprintf(mouseStr, "Position: (%d, %d)  Buttons: [%c][%c]", 
                mouseX, mouseY, 
                leftDown ? 'L' : ' ', 
                rightDown ? 'R' : ' ');
        outtextxy(40, 130, mouseStr);

        // 绘制退出提示
        outtextxy(20, 400, "Press ESC to exit");

        // 刷新显示
        FlushBatchDraw();
        Sleep(10);
    }

    closegraph();
    return 0;
}
