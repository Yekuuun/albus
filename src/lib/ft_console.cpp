#include "ft_console.hpp"

/**
 * Changing cmd color
 * @param color => color code.
 */
VOID Ft_console::SetBashColor(IN WORD wColor){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsole)
        SetConsoleTextAttribute(hConsole, wColor);
}