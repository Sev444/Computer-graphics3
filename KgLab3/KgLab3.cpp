#include <windows.h>
#include <math.h>
#include <vector>
#include <queue>
#define PI 3.1415

struct Point {
	float x, y;
};
struct Figure {
	std::vector<Point> points;
} figure;


void transfer(float Dx, float Dy) {
	for (auto i = 0; i < figure.points.size(); i++)
		figure.points[i].x += Dx, figure.points[i].y += Dy;
}
void scale(float Sx, float Sy) {
	float center_x = 0, center_y = 0;
	for (auto i = 0; i < figure.points.size(); i++)
		center_x += figure.points[i].x, center_y += figure.points[i].y;
	center_x /= figure.points.size(), center_y /= figure.points.size();

	for (auto i = 0; i < figure.points.size(); i++) {
		figure.points[i].x = center_x + (figure.points[i].x - center_x) * Sx;
		figure.points[i].y = center_y + (figure.points[i].y - center_y) * Sy;
	}
}
void rotate(float angle) {
	float center_x = 0, center_y = 0;
	for (auto i = 0; i < figure.points.size(); i++)
		center_x += figure.points[i].x, center_y += figure.points[i].y;
	center_x /= figure.points.size(), center_y /= figure.points.size();

	for (auto i = 0; i < figure.points.size(); i++) {
		figure.points[i].x = center_x + (figure.points[i].x - center_x) * cos(angle * PI / 180)
			- (figure.points[i].y - center_y) * sin(angle * PI / 180);
		figure.points[i].y = center_y + (figure.points[i].x - center_x) * sin(angle * PI / 180)
			+ (figure.points[i].y - center_y) * cos(angle * PI / 180);
	}
}
void regular_polygon(float x, float y, float R) {
	float angle = 0;
	for (auto i = 0; i < figure.points.size(); i++) {
		figure.points[i].x = x + R * cos(angle * PI / 180);
		figure.points[i].y = y + R * sin(angle * PI / 180);
		angle = angle + 360 / figure.points.size();
	}
}

void line_b(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
	int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	while (1) {
		SetPixel(hdc, x1, y1, color);
		if (x1 == x2 && y1 == y2)
			break;
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y1 += sy;
		}
	}
}

void draw(HDC hdc, COLORREF color) {
	HPEN hPen;
	hPen = CreatePen(PS_DASHDOT, 2, color);
	SelectObject(hdc, hPen);
	MoveToEx(hdc, figure.points[0].x, figure.points[0].y, NULL);
	for (auto i = 1; i < figure.points.size(); i++)
		LineTo(hdc, figure.points[i].x, figure.points[i].y);
	LineTo(hdc, figure.points[0].x, figure.points[0].y);
	DeleteObject(hPen);
}
void draw_bresenham(HDC hdc, COLORREF color) {
	for (size_t i = 0; i < figure.points.size(); i++) {
		size_t j = i < figure.points.size() - 1 ? i + 1 : 0;
		line_b(hdc, figure.points[i].x, figure.points[i].y,
			figure.points[j].x, figure.points[j].y, color);
	}
}
void draw_painted(HDC hdc, COLORREF color) {
	draw(hdc, color);

	float center_x = 0, center_y = 0;
	for (auto i = 0; i < figure.points.size(); i++)
		center_x += figure.points[i].x, center_y += figure.points[i].y;
	center_x /= figure.points.size(), center_y /= figure.points.size();

	POINT pxl = { (long)center_x,  (long)center_y };
	std::queue <POINT> pixels;
	pixels.push(pxl);
	while (!pixels.empty()) {
		pxl = pixels.front(), pixels.pop();
		COLORREF pixelcolor = GetPixel(hdc, pxl.x, pxl.y);
		if (pixelcolor == CLR_INVALID || pixelcolor == color)
			continue;
		else {
			SetPixel(hdc, pxl.x, pxl.y, color);
			--pxl.y, pixels.push(pxl);
			pxl.y += 2, pixels.push(pxl);
			--pxl.y, --pxl.x, pixels.push(pxl);
			pxl.x += 2, pixels.push(pxl);
		}
	}
}

void control(int key) {
	if (key == int('W')) transfer(0, -2);
	if (key == int('A')) transfer(-2, 0);
	if (key == int('S')) transfer(0, 2);
	if (key == int('D')) transfer(2, 0);
	if (key == int('X')) scale(1.02, 1.02);
	if (key == int('Z')) scale(0.98, 0.98);
	if (key == int('E')) rotate(2);
	if (key == int('Q')) rotate(-2);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps; RECT Rect; HDC hdc, hCmpDC; HBITMAP hBmp;

	switch (messg) {

	case WM_PAINT: {
		GetClientRect(hWnd, &Rect);
		hdc = BeginPaint(hWnd, &ps);

		// Создание теневого контекста для двойной буферизации
		hCmpDC = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, Rect.right - Rect.left, Rect.bottom - Rect.top);
		SelectObject(hCmpDC, hBmp);

		// Закраска фоновым цветом
		LOGBRUSH br;
		br.lbStyle = BS_SOLID;
		br.lbColor = 0xFFFFFF;
		HBRUSH brush;
		brush = CreateBrushIndirect(&br);
		FillRect(hCmpDC, &Rect, brush);
		DeleteObject(brush);

		// Отрисовка
		//draw(hCmpDC, 0x0000FF);
		//draw_bresenham(hCmpDC, 0x0000FF);
		draw_painted(hCmpDC, 0x0000FF);

		// Копируем изображение из теневого контекста на экран
		SetStretchBltMode(hdc, COLORONCOLOR);
		BitBlt(hdc, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, hCmpDC, 0, 0, SRCCOPY);

		// Удаляем ненужные системные объекты
		DeleteDC(hCmpDC);
		DeleteObject(hBmp);
		hCmpDC = NULL;

		EndPaint(hWnd, &ps);
	} break;

	case WM_ERASEBKGND:
		return 1;
		break;

	case WM_KEYDOWN:
		if (GetKeyState(int('C')))
			control(int(wParam));
		else
			control(int(wParam));
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return (DefWindowProc(hWnd, messg, wParam, lParam));
	}
	return (0);
}
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	HWND hWnd;
	MSG lpMsg;
	WNDCLASS WndClass;

	figure.points.resize(5);
	regular_polygon(500, 250, 150);

	// Заполняем структуру класса окна
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = L"CG_WAPI_Template";

	// Регистрируем класс окна
	if (!RegisterClass(&WndClass)) {
		MessageBox(NULL, L"Не могу зарегистрировать класс окна!", L"Ошибка", MB_OK);
		return 0;
	}

	// Создаем основное окно приложения
	hWnd = CreateWindow(
		L"CG_WAPI_Template", // Имя класса 
		L"Компьютерная графика лабораторная работа №2", // Текст заголовка
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // Стиль окна 
		CW_USEDEFAULT, CW_USEDEFAULT, // Позиция левого верхнего угла 
		1000, 500, // Ширина и высота окна 
		(HWND)NULL, // Указатель на родительское окно NULL 
		(HMENU)NULL, // Используется меню класса окна 
		(HINSTANCE)hInstance, // Указатель на текущее приложение
		NULL); // Передается в качестве lParam в событие WM_CREATE


	// Если окно не удалось создать
	if (!hWnd) {
		MessageBox(NULL, L"Не удается создать главное окно!", L"Ошибка", MB_OK);
		return 0;
	}

	// Показываем окно
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Выполняем цикл обработки сообщений до закрытия приложения
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	return (int)lpMsg.wParam;
}