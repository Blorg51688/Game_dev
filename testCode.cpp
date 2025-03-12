#include <graphics.h>
#include <conio.h>
#include <iostream>

int main()
{
    // ��ʼ��ͼ�δ���
    initgraph(640, 480);
    setbkcolor(WHITE);
    cleardevice();
    
    // ���ÿ���̨�������ڵ������
    HWND hwnd = GetForegroundWindow();
    ShowWindow(hwnd, SW_SHOW);
    
    // ����״̬����
    int keyCode = -1;
    char keyChar = 0;
    int mouseX = 0, mouseY = 0;
    bool leftDown = false;
    bool rightDown = false;

    // ��ѭ��
    while (true)
    {
        // �����������
        if (_kbhit())
        {
            keyCode = _getch();
            keyChar = (keyCode >= 32 && keyCode <= 126) ? keyCode : 0;
            
            // �������������̨
            std::cout << "Key detected: " << keyCode 
                      << " (" << keyChar << ")\n";
            
            if (keyCode == 27) break; // ESC�˳�
        }

        // �����������
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

        // ͼ�ν������
        cleardevice();
        
        // ���Ƽ���״̬
        settextcolor(BLACK);
        outtextxy(20, 20, "Keyboard Status:");
        char keyStr[50];
        sprintf(keyStr, "Code: %d  Char: %c", keyCode, keyChar);
        outtextxy(40, 50, keyStr);

        // �������״̬
        outtextxy(20, 100, "Mouse Status:");
        char mouseStr[100];
        sprintf(mouseStr, "Position: (%d, %d)  Buttons: [%c][%c]", 
                mouseX, mouseY, 
                leftDown ? 'L' : ' ', 
                rightDown ? 'R' : ' ');
        outtextxy(40, 130, mouseStr);

        // �����˳���ʾ
        outtextxy(20, 400, "Press ESC to exit");

        // ˢ����ʾ
        FlushBatchDraw();
        Sleep(10);
    }

    closegraph();
    return 0;
}
