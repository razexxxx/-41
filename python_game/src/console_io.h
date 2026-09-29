// console_io.h - неблокирующий ввод с клавиатуры (кроссплатформенно: Linux termios / Windows WinAPI)
#ifndef CONSOLE_IO_H
#define CONSOLE_IO_H

#include <string>

void enableRawMode();
void disableRawMode();

// Неблокирующее чтение одного «символа» ввода.
// Стрелки ANSI (ESC [ A/B/C/D) распознаются и возвращаются как спецкоды:
//   '\x01' вверх, '\x02' вниз, '\x03' влево, '\x04' вправо
// Возвращает '\0', если доступного ввода нет.
char readKeyNonBlocking();

void clearScreen();
void printUtf8(const std::string& s); // печать строки с короткой паузой отрисовки

#endif // CONSOLE_IO_H
