#pragma once

#include "ft_pe.hpp"

class Pe;

/**
 * Base struct for hash table.
 */
typedef struct BUILTINS_PE {
    CHAR *cName;
    VOID (Pe::*func)(CHAR **args);
    Pe *instance;
} BUILTINS_PE, *PBUILTINS_PE;

/**
 * Base class.
 */
class Pe {
    private:
        //private vars.
        LPCSTR lpPath = nullptr;
        PBYTE  pBuff  = nullptr;
        PBYTE  pRawPe = nullptr;
        DWORD  dwSizeOfImage = 0;

        //handling builtins commands.
        PBUILTINS_PE builtins[10] = {0};

        //handlers.
        UINT HashFunction(IN CHAR *cName);
        VOID AddBuiltin(IN CHAR *cName, IN VOID (Pe::*func)(CHAR **));
        VOID Exec(IN CHAR **args);
        BOOL IsBuiltin(IN CHAR *cName);

        //builtins.
        VOID Unload(IN CHAR** = nullptr);
        VOID Infos(IN CHAR** = nullptr);
        VOID Load(IN CHAR **args);
        VOID Dump(IN CHAR** args);

    public:
        VOID HandleCommand(IN CHAR **args);
        VOID Clean();
        VOID Init();

        Pe();
        ~Pe();
};