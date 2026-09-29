// console_io.cpp - неблокирующий ввод с клавиатуры (кроссплатформенно: Linux/Windows)
#include "console_io.h"

#include <cstdio>
#include <chrono>
#include <thread>

#ifdef _WIN32
// ============================ WINDOWS ============================
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static bool g_rawEnabled   = false;
static HANDLE g_hIn  = nullptr;
static DWORD  g_origMode = 0;

void enableRawMode() {
    if (g_rawEnabled) return;
    g_hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (g_hIn == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleMode(g_hIn, &g_origMode)) return;
    // отключаем эхо, processed input и quick-edit (блокирует консоль при клике)
    DWORD mode = g_origMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
    mode |= ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(g_hIn, mode);
    // UTF-8 в консоли для эмодзи
    SetConsoleOutputCP(65001);
    SetConsoleInputCP(65001);
    g_rawEnabled = true;
}

void disableRawMode() {
    if (!g_rawEnabled) return;
    SetConsoleMode(g_hIn, g_origMode);
    g_rawEnabled = false;
}

char readKeyNonBlocking() {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    for (;;) {
        DWORD avail = 0;
        if (!GetNumberOfConsoleInputEvents(hIn, &avail) || avail == 0) return '\0';
        INPUT_RECORD rec;
        DWORD read_ = 0;
        if (!ReadConsoleInputA(hIn, &rec, 1, &read_)) return '\0';
        if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown) continue;

        WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
        switch (vk) {
            case VK_UP:    return '\x01';
            case VK_DOWN:  return '\x02';
            case VK_LEFT:  return '\x03';
            case VK_RIGHT: return '\x04';
        }
        char ch = rec.Event.KeyEvent.uChar.AsciiChar;
        if (ch != 0) return ch;
        // прочитали служебное событие без символа - пробуем следующее
    }
}

void clearScreen() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(h, &csbi)) {
        DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
        DWORD written = 0;
        COORD home = {0, 0};
        FillConsoleOutputCharacterA(h, ' ', cells, home, &written);
        SetConsoleCursorPosition(h, home);
    } else {
        std::fputs("\033[2J\033[H", stdout);
    }
    std::fflush(stdout);
}

void printUtf8(const std::string& s) {
    std::fwrite(s.data(), 1, s.size(), stdout);
    std::fflush(stdout);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

#else
// ============================ LINUX / UNIX ============================
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

static termios g_origTermios;
static bool    g_rawEnabled = false;

void enableRawMode() {
    if (g_rawEnabled) return;
    if (tcgetattr(STDIN_FILENO, &g_origTermios) != 0) return;
    termios raw = g_origTermios;
    raw.c_lflag &= ~(ECHO | ICANON); // без эха и построчной буферизации
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    int fl = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (fl >= 0) fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK);
    g_rawEnabled = true;
}

void disableRawMode() {
    if (!g_rawEnabled) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_origTermios);
    int fl = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (fl >= 0) fcntl(STDIN_FILENO, F_SETFL, fl & ~O_NONBLOCK);
    g_rawEnabled = false;
}

char readKeyNonBlocking() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return '\0';
    if (c == '\033') { // экранированная последовательность (стрелки)
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\0';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\0';
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return '\x01'; // вверх
                case 'B': return '\x02'; // вниз
                case 'C': return '\x04'; // вправо
                case 'D': return '\x03'; // влево
            }
        }
        return '\0';
    }
    return c;
}

void clearScreen() {
    std::fputs("\033[2J\033[H", stdout);
    std::fflush(stdout);
}

void printUtf8(const std::string& s) {
    std::fwrite(s.data(), 1, s.size(), stdout);
    std::fflush(stdout);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

#endif // _WIN32
