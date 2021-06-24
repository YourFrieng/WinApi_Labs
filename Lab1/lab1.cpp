#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

LRESULT CALLBACK WindowProcedure(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);

class WinClass
{
public:

    WinClass(WNDPROC winProc, wchar_t const* className, HINSTANCE hInst);
    void Register()
    {

        ::RegisterClass(&_class);

    }

private:

    WNDCLASS _class;

};
WinClass::WinClass(WNDPROC winProc, wchar_t const* className, HINSTANCE hInst)

{

    _class.style = 0;
    _class.lpfnWndProc = winProc; // віконна процедура: обов'язкова
    _class.cbClsExtra = 0;
    _class.cbWndExtra = 0;
    _class.hInstance = hInst;         // власник класу: обов'язковий
    _class.hIcon = 0;
    _class.hCursor = ::LoadCursor(0, IDC_CROSS /*IDC_ARROW*/); // optional
    _class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // optional
    _class.lpszMenuName = 0;
    _class.lpszClassName = className; // обов'язково


}

class WinMaker
{
public:

    WinMaker() : _hwnd(0) {}
    WinMaker(wchar_t const* caption, wchar_t const* className, HINSTANCE hInstance);
    void Show(int cmdShow)
    {

        ::ShowWindow(_hwnd, cmdShow);
        ::UpdateWindow(_hwnd);

    }

protected:

    HWND _hwnd;

};

WinMaker::WinMaker(wchar_t const* caption, wchar_t const* className, HINSTANCE hInstance)
{

    _hwnd = ::CreateWindow(

        className,            // ім'я реєструємого віконного класу
        caption,              // заголовок вікна
        WS_OVERLAPPEDWINDOW,  // стиль вікна
        CW_USEDEFAULT,        // позиція x
        CW_USEDEFAULT,        // позиція y
        CW_USEDEFAULT,        // ширина
        CW_USEDEFAULT,        // висота
        0,                    // handle батьківського вікна
        0,                    // handle до меню
        hInstance,            // дескриптор екземпляра
        0);                   // дата створення (window creation data)

}



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char* cmdParam, int cmdShow)

{

    wchar_t className[] = L"Winnie";

    WinClass winClass(WindowProcedure, className, hInst);
    winClass.Register();

    WinMaker win(L"Hello Windows!", className, hInst);
    win.Show(cmdShow);

    MSG msg;
    int status;

    while ((status = ::GetMessage(&msg, 0, 0, 0)) != 0)
    {
        TranslateMessage(&msg);
        if (status == -1)
            return -1;

        ::DispatchMessage(&msg);

    }
    return msg.wParam;

}


LRESULT CALLBACK WindowProcedure(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    //шрифт вывода в окно
    static HFONT hfont = NULL;

    HDC hdc;
    static HWND hEdt1;
    static HWND hBtn; // дескриптор кнопки
    static HWND hStat; // дескриптор статического текста
    HINSTANCE hInst;
    PAINTSTRUCT ps;
    wchar_t const* windowText = L"Hello, world!";
    int Len;
    int height = 12;
    wchar_t StrA[4];


    

    switch (message)
    {
    case WM_CREATE:
        hInst = ((LPCREATESTRUCT)lParam)->hInstance;

        hEdt1 = CreateWindow(L"EDIT", L"0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_RIGHT, 150, 5, 120, 30,
            hwnd, 0, hInst, NULL);
        ShowWindow(hEdt1, SW_SHOWNORMAL);

        hBtn = CreateWindow(L"button", L"Изменить",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            300, 5, 120, 30, hwnd, 0, hInst, NULL);
        ShowWindow(hBtn, SW_SHOWNORMAL);
        //TODO make in static box text "Now letter height:"
        hStat = CreateWindow(L"static", L"90", WS_CHILD | WS_VISIBLE | ES_RIGHT,
            5, 5, 120, 30, hwnd, 0, hInst, NULL);
        ShowWindow(hStat, SW_SHOWNORMAL);

        hfont = CreateFont(90, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                            DEFAULT_QUALITY, VARIABLE_PITCH, L"Times New Roman");
        break;
        //TODO make read if enter key
    /*case WM_KEYDOWN://нажата клавиша
        switch (wParam)
        {
            
        case VK_CONTROL://клавиша контрол
            Len = GetWindowText(hEdt1, StrA, 4);
            height = _wtoi(StrA);
            SetWindowText(hStat, StrA);
            if (hfont) DeleteObject(hfont);
            hfont = CreateFont(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, VARIABLE_PITCH, L"Times New Roman");
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
            break;
        }
        return 0;*/
    case WM_COMMAND: 
        if (lParam == (LPARAM)hBtn)    // если нажали на кнопку ENTER
        {
            Len = GetWindowText(hEdt1, StrA, 4);
            height = _wtoi(StrA);
            SetWindowText(hStat, StrA);
            if (hfont) DeleteObject(hfont);
            hfont = CreateFont(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, VARIABLE_PITCH, L"Times New Roman");
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
            
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, hfont);
        TextOut(hdc, 5, 50, windowText, _tcslen(windowText));
        EndPaint(hwnd, &ps);
        break;
    
    case WM_DESTROY:

        ::PostQuitMessage(0);
        if (hfont) DeleteObject(hfont);
        return 0;

    }
    return ::DefWindowProc(hwnd, message, wParam, lParam);

}
