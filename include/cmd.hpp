/**
 * Author : Yekuuun
 * Github : https://github.com/Yekuuun
 * 
 * Notes  : this class contains base code implementation of the inner albus shell who contains pre-builtins commands
 *          for better interactivity.
 */

#pragma once

#include "global.hpp"

#define MAX_TOKENS_LEN 100

class Cmd;

/**
 * Base struct for hash table.
 */
typedef struct BUILTINS {
    CHAR *cName;
    VOID (Cmd::*func)(CHAR **args);
    Cmd *instance;
} BUILTINS, *PBUILTINS;

/**
 * Token struct
 */
typedef struct TOKEN {
    DWORD dwIndex;
    CHAR *cmd;
    struct TOKEN *next;
} TOKEN, *PTOKEN;


/**
 * Main CMD class in charge of running Albus.
 */
class Cmd {
    private:
        //handling running state.
        INT KEEP_RUNNING;

        //handling builtins commands.
        PBUILTINS builtins[10] = {0};

        //tokens list.
        PTOKEN tokens = nullptr;

        //init
        VOID ShowBanner();

        //main private functions.
        static BOOL WINAPI HandleCtrlC(IN DWORD dwType);
        VOID SetBashColor(IN WORD wColor); 
        VOID DisplayCmdHeader(); 
        
        //shell handlers.
        UINT HashFunction(IN CHAR *cName);
        VOID AddBuiltin(IN CHAR *cName, IN VOID (Cmd::*func)(CHAR **));
        VOID Exec(IN CHAR **args);
        BOOL IsBuiltin(IN CHAR *cName);
        VOID Init();

        //bultins commands.
        VOID Clean(IN CHAR** = nullptr);
        VOID Pwd(IN CHAR** = nullptr);
        VOID Exit(IN CHAR **args);
        VOID Help(IN CHAR** = nullptr);

        //parsing & lexing
        VOID Lexer(IN CHAR *cInput);
        PTOKEN CreateToken(IN CHAR *cmd, IN DWORD dwIndex);
        VOID AddToken(IN PTOKEN *head, IN CHAR *cmd, IN DWORD dwIndex);
        VOID FreeTokens(IN PTOKEN head);

        //handle commands
        CHAR **ParseCommands(IN PTOKEN tokens, OUT WORD *argc);

    public:
        //constructors.
        Cmd();
        ~Cmd();

        //running shell.
        VOID RunShell();  
};