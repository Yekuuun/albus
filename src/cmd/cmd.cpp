#include "cmd.hpp";

Cmd::Cmd(){};
Cmd::~Cmd(){}

/**
 * Handling CTRL-C commands.
 * @param dwType => event type.
 */
BOOL WINAPI Cmd::HandleCtrlC(IN DWORD dwType){
    return dwType == CTRL_C_EVENT;
}

/**
 * Changing cmd color
 * @param color => color code.
 */
VOID Cmd::SetBashColor(IN WORD wColor){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsole)
        SetConsoleTextAttribute(hConsole, wColor);
}

/**
 * Display cmd header.
 */
VOID Cmd::DisplayCmdHeader(){
    this->SetBashColor(10);
    printf("albus > ");
    this->SetBashColor(7);
}

//-------SHELL HANDLERS-------

/**
 * Hash function creating an unique index.
 * @param cName => in function name.
 */
UINT Cmd::HashFunction(IN CHAR *cName){
    UINT hash = 0;
    while(*cName){
        hash += *cName++;
    }

    return hash % HASH_TABLE_BUILTINS_SIZE;
}

/**
 * Add a function to builtins table.
 * @param cName => name of cmd
 * @param func  => function ptr.
 */
VOID Cmd::AddBuiltin(IN CHAR *cName, IN VOID (Cmd::*func)(CHAR **)){
    UINT index    = this->HashFunction(cName);
    PBUILTINS cmd = (PBUILTINS)malloc(sizeof(BUILTINS));
    if(cmd == NULL)
        return;
    
    cmd->cName    = cName;
    cmd->func     = func;
    cmd->instance = this; // Stocke l'instance de la classe
    this->builtins[index] = cmd;
}

/**
 * Exec builtins function.
 * @param args => list of arguments.
 */
VOID Cmd::Exec(IN CHAR **args){
    UINT index = this->HashFunction(args[0]);
    if (this->builtins[index] && strcmp(this->builtins[index]->cName, args[0]) == 0) {
        PBUILTINS cmd = this->builtins[index];
        (cmd->instance->*(cmd->func))(args); // Appel correct de la méthode membre
    }
}

/**
 * Check if cmd is in BUILTINS commands.ABC
 * @params cStr => cCommand.
 */
BOOL Cmd::IsBuiltin(IN CHAR *cStr){
    UINT index = this->HashFunction(cStr);
    PBUILTINS cmd = this->builtins[index];

    return this->builtins[index] != NULL && (strcmp(this->builtins[index]->cName, cStr) == 0);
}


/**
 * Init builtins.
 */
VOID Cmd::Init(){
    this->AddBuiltin("clean", &Cmd::Clean);
    this->AddBuiltin("pwd", &Cmd::Pwd);
    this->AddBuiltin("exit", &Cmd::Exit);
}


//-------BUILTINS------

/**
 * Cleaning CMD.
 */
VOID Cmd::Clean(CHAR**){
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == NULL)
        return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hStdOut, &dwMode))
        return;

    const DWORD dwOriginalMode = dwMode;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(hStdOut, dwMode))
        return;

    DWORD dwWritten = 0;
    PCWSTR pwSeq = L"\x1b[2J\x1b[H";  // Efface et remet le curseur en haut

    if (!WriteConsoleW(hStdOut, pwSeq, (DWORD)wcslen(pwSeq), &dwWritten, NULL)) {
        SetConsoleMode(hStdOut, dwOriginalMode);
        return;
    }

    SetConsoleMode(hStdOut, dwOriginalMode);
}

/**
 * Display current dir absolute path.
 */
VOID Cmd::Pwd(CHAR**){
    LPSTR lpPath = (LPSTR)malloc(MAX_PATH * sizeof(char));
    if(lpPath == NULL)
        return;

    DWORD dwPath = GetCurrentDirectoryA(MAX_PATH, lpPath);
    if(dwPath != 0)
        printf("%s\n", lpPath);
}

/**
 * Base exit command.
 * @param args => exit code for exit command.
 */
VOID Cmd::Exit(IN CHAR **args){
    INT EXIT_CODE = 0;

    if(args[1] != NULL)
        EXIT_CODE = atoi(args[1]);

    printf("$ Exiting severus. CIAO....\n");
    exit(EXIT_CODE);
}


//--------------------------------------------------

/**
 * Main function in charge of running shell handler.
 */
VOID Cmd::RunShell(){
    CHAR cBuffer[MAX_TOKENS_LEN] = {0};

    this->Init();
    this->Clean();

    if(!SetConsoleCtrlHandler(&Cmd::HandleCtrlC, TRUE)){
        this->Exit(nullptr);
    }

    while(this->KEEP_RUNNING){
        this->DisplayCmdHeader();

        if(fgets(cBuffer, sizeof(cBuffer), stdin) != NULL){
            cBuffer[strcspn(cBuffer, "\n")] = '\0';
            printf("%s\n", cBuffer);

            Sleep(1);
        }
        else {
            this->KEEP_RUNNING = 0;
        }
    }

    return;
}
