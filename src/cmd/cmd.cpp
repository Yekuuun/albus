/**
 * Author : Yekuuunu
 * Github : https://github.com/Yekuuun
 * 
 * Notes  : this file contains the base implementation of the inner minishell for albus. (command interpreter)
 */

#include "cmd.hpp";
#include "ft_str.hpp";

Cmd::Cmd(){};
Cmd::~Cmd(){};

/**
 * Handling CTRL-C commands.
 * @param dwType => event type.
 */
BOOL WINAPI Cmd::HandleCtrlC(IN DWORD dwType){
    return dwType == CTRL_C_EVENT;
}

/**
 * Show albus banner.
 */
VOID Cmd::ShowBanner(){
    std::cout << "\033[1;32m" // Active la couleur verte
    << R"(
        ___    ____              
       /   |  / / /_  __  _______
      / /| | / / __ \/ / / / ___/
     / ___ |/ / /_/ / /_/ (__  ) 
    /_/  |_/_/_.___/\__,_/____/  
                        
    )" 
    << "\033[0m" << std::endl; // Réinitialise la couleur


    std::cout <<
    "\033[1;33m-------------------------------------------\n"
    "\033[1;32mProject:  \033[1;34mAlbus\n"
    "\033[1;32mAuthor:   \033[1;34mYekuuun\n"
    "\033[1;32mVersion:  \033[1;34m1.0.0\n"
    "\033[1;32mDescription: \033[1;34mA base CLI PE viewer\n"
    "\033[1;33m-------------------------------------------\n"
    "\033[0m"
    << std::endl;
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
    if(cmd == nullptr)
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

    return this->builtins[index] != nullptr && (strcmp(this->builtins[index]->cName, cStr) == 0);
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
VOID Cmd::Clean(IN CHAR**){
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == nullptr)
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
VOID Cmd::Pwd(IN CHAR**){
    LPSTR lpPath = (LPSTR)malloc(MAX_PATH * sizeof(char));
    if(lpPath == NULL)
        return;

    DWORD dwPath = GetCurrentDirectoryA(MAX_PATH, lpPath);
    if(dwPath != 0)
        printf("$%s\n", lpPath);
}

/**
 * Base exit command.
 * @param args => exit code for exit command.
 */
VOID Cmd::Exit(IN CHAR **args){
    INT EXIT_CODE = 0;

    if(args[1] != NULL)
        EXIT_CODE = atoi(args[1]);

    printf("$ Exiting albus. CIAO....\n");

    if(this->tokens)
        this->FreeTokens(this->tokens);
        
    exit(EXIT_CODE);
}

/**
 * Base help command.
 */
VOID Cmd::Help(IN CHAR**){
    std::cout << "ALBUS COMMANDS :" << std::endl;
    printf("\n");

    std::cout << "SYNOPSIS :" << std::endl;
    printf("\t [COMMAND]... [OPTIONS]...\n");
}

//--------LEXING & PARSING-----------

/**
 * Creating a new token.
 * @param cmd => token
 * @param dwIndex => index => (count)
 */
PTOKEN Cmd::CreateToken(IN CHAR *cmd, IN DWORD dwIndex){
    PTOKEN token = (PTOKEN)malloc(sizeof(TOKEN));
    if(token == nullptr)
        return nullptr;
    
    token->cmd = Ft_str::Ft_Strndup(cmd, strlen(cmd));
    token->dwIndex = dwIndex;
    token->next = nullptr;

    return token;
}

/**
 * Adding a new token to the linked list.
 * @param cmd => token
 * @param dwIndex => index => (count)
 * @param head => beginning of the list.
 */
VOID Cmd::AddToken(IN PTOKEN *head, IN CHAR *cmd, IN DWORD dwIndex){
    PTOKEN newToken = this->CreateToken(cmd, dwIndex);
    if(newToken == nullptr)
        return; 
    
    if (*head == nullptr) {
        *head = newToken;
    }
    else {
        PTOKEN temp = *head;
        while(temp->next)
            temp = temp->next;
        
        temp->next = newToken;
    }
}

/**
 * Lexing entry of inputs.
 */
VOID Cmd::Lexer(IN CHAR *cInput){
    DWORD dwIndex = 0;
    DWORD i = 0;

    this->tokens = nullptr;

    while(cInput[i]){
        CHAR cCurrent = cInput[i];

        if(isspace(cCurrent)){
            i++;
            continue;
        }
        else if(cInput[i] == '\0'){
            break;
        }
        else {
            DWORD dwStart = i;
            while(cInput[i] && !isspace(cInput[i]))
                i++;
            
            CHAR *cWord = Ft_str::Ft_Strndup(&cInput[dwStart], i - dwStart);
            if(cWord)
                AddToken(&this->tokens, cWord, dwIndex);
            
            dwIndex++;
        }
    }
}

/**
 * Free list of tokens.
 */
VOID Cmd::FreeTokens(IN PTOKEN head){
    while(head){
        PTOKEN next = head->next;
        free(head->cmd);
        free(head);
        head = next;
    }

    head = nullptr;
}

/**
 * Parsing commands.
 */
CHAR** Cmd::ParseCommands(IN PTOKEN tokens, OUT WORD *argc){
    CHAR **result  = nullptr;
    WORD i         = 0;
    *argc          = 0;

    PTOKEN current = tokens;

    if(tokens == nullptr)
        return nullptr;
    
    while(current){
        (*argc)++;
        current = current->next;
    }

    current = tokens;
    result = (CHAR**)malloc((*argc + 1) * sizeof(CHAR*));
    if(result == nullptr)
        return nullptr;

    while(current){
        result[i] = current->cmd;
        current = current->next;
        i++;
    }

    result[*argc] = NULL;
    return result;
}


//--------------------------------------------------

/**
 * Main function in charge of running shell handler.
 */
VOID Cmd::RunShell(){

    //cmd handling.
    CHAR **cmd = nullptr;
    WORD argc  = 0;

    //fgets handler.
    CHAR cBuffer[MAX_TOKENS_LEN] = {0};

    this->Init();
    this->Clean();

    Sleep(1);
    this->ShowBanner();

    if(!SetConsoleCtrlHandler(Cmd::HandleCtrlC, TRUE)){
        this->Exit(nullptr);
    }

    //main loop.
    while(this->KEEP_RUNNING){
        this->DisplayCmdHeader();

        if(fgets(cBuffer, sizeof(cBuffer), stdin) != nullptr){
            cBuffer[strcspn(cBuffer, "\n")] = '\0';

            //freeing mem
            if(this->tokens)
                this->FreeTokens(tokens);
            
            if(cmd)
                free(cmd);
            //------------
            
            this->Lexer(cBuffer);

            //handling command.
            cmd = this->ParseCommands(this->tokens, &argc);
            if(cmd == nullptr){
                std::cout << "[!] An error occured." << std::endl;
                break;
            }

            if(argc == 0)
                continue;
            
            if(this->IsBuiltin(cmd[0])){
                this->Exec(cmd);
            }

            Sleep(1);
        }
        else {
            this->KEEP_RUNNING = 0;
        }
    }

__CLEAN:
    if(this->tokens)
        this->FreeTokens(tokens);

    return;
}
