/**
 * Author : Yekuuun
 * Github : https://github.com/Yekuuun
 * 
 * This class implements the base business logic for manipulating PE from MEMORY.
 */

#include "pe.hpp"

Pe::Pe(){};
Pe::~Pe(){};

//-----HANDLERS-----
/**
 * Hash function creating an unique index.
 * @param cName => in function name.
 */
UINT Pe::HashFunction(IN CHAR *cName) {
    UINT hash = 5381;
    while (*cName) {
        hash = ((hash << 5) + hash) + *cName++; // hash * 33 + c
    }
    return hash % HASH_TABLE_BUILTINS_SIZE;
};

/**
 * Add a function to builtins table.
 * @param cName => name of cmd
 * @param func  => function ptr.
 */
VOID Pe::AddBuiltin(IN CHAR *cName, IN VOID (Pe::*func)(CHAR **)){
    UINT index    = this->HashFunction(cName);
    PBUILTINS_PE cmd = (PBUILTINS_PE)malloc(sizeof(BUILTINS_PE));
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
VOID Pe::Exec(IN CHAR **args){
    UINT index = this->HashFunction(args[0]);
    if (this->builtins[index] && strcmp(this->builtins[index]->cName, args[0]) == 0) {
        PBUILTINS_PE cmd = this->builtins[index];
        (cmd->instance->*(cmd->func))(args); // Appel correct de la méthode membre
    }
}

/**
 * Check if cmd is in BUILTINS commands.ABC
 * @params cStr => cCommand.
 */
BOOL Pe::IsBuiltin(IN CHAR *cStr){
    UINT index = this->HashFunction(cStr);
    PBUILTINS_PE cmd = this->builtins[index];

    return this->builtins[index] != nullptr && (strcmp(this->builtins[index]->cName, cStr) == 0);
}


/**
 * Init builtins.
 */
VOID Pe::Init(){
    this->AddBuiltin("load", &Pe::Load);
    this->AddBuiltin("unload", &Pe::Unload);
    this->AddBuiltin("infos", &Pe::Infos);
}

/**
 * Clean memory for exit.
 */
VOID Pe::Clean(){
    this->dwSizeOfImage = 0;

    if(this->pBuff)
        VirtualFree(this->pBuff, this->dwSizeOfImage, MEM_RELEASE);

    if(this->lpPath)
        free((VOID*)this->lpPath);

    if(this->pRawPe)
        HeapFree(GetProcessHeap(), 0, this->pRawPe);
}

/**
 * Handle et exec PE command.
 */
VOID Pe::HandleCommand(IN CHAR **args){
    if(args[0] == nullptr){
        std::cout << "$ Please, use a valid command. use help for more informations." << std::endl;
        return;
    }

    if(!this->IsBuiltin(args[0])){
        std::cout << "$ Please, use a valid command. use help for more informations." << std::endl;
        return;
    }

    this->Exec(args);
}

//-------------------------------------------

/**
 * Show informations about current PE loaded.
 */
VOID Pe::Infos(IN CHAR**){
    if(this->pBuff){
        std::cout << "------------------------" << std::endl;
        std::cout << "Name :" << this->lpPath << std::endl;
        std::cout << "Image size : " << this->dwSizeOfImage << std::endl;
        std::cout << "Base address : " << "0x" << static_cast<void*>(this->pBuff) << std::endl;
        std::cout << "------------------------" << std::endl;
        printf("\n"); 
    }
    else {
        std::cout << "$ No PE loaded." << std::endl;
    }
}

/**
 * Unload PE from memory.
 */
VOID Pe::Unload(IN CHAR**){
    if(this->pBuff == nullptr){
        std::cout << "$ No PE mapped. try using help command for mapping a file." << std::endl;
    }
    else {        
        VirtualFree(this->pBuff, this->dwSizeOfImage, MEM_RELEASE);
        std::cout << "$ Successfully unloaded " << this->lpPath << std::endl;

        this->dwSizeOfImage = 0;

        if(this->lpPath)
            free((VOID*)this->lpPath);

        HeapFree(GetProcessHeap(), 0, this->pRawPe);
    }
}

/**
 * Load PE into memory.
 */
VOID Pe::Load(IN CHAR **args){
    if(args[0] == nullptr || args[1] == nullptr){
        std::cout << "$ load accept one argument <path_to_pe_file>." << std::endl;
        return;
    }

    CHAR *cPath  = args[1];
    DWORD dwSize = 0;

    this->pRawPe = ReadPeFile(cPath, &dwSize);
    if(dwSize == INVALID_FILE_SIZE || this->pRawPe == nullptr)
        return;
    
    std::cout << "$ Read " << dwSize << " bytes of file" << cPath << std::endl;
    
    //load Pe.
    PIMAGE_DOS_HEADER pDos   = (PIMAGE_DOS_HEADER)this->pRawPe;
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)((BYTE*)this->pRawPe + pDos->e_lfanew);

    this->pBuff = (PBYTE)VirtualAlloc(NULL, pNtHdr->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if(this->pBuff == nullptr){
        std::cout << "$ Error allocating memory." << std::endl;
        goto __ERROR;
    }

    printf("$ Memory allocated at 0x%p \n", pBuff);

    MapSections(this->pRawPe, this->pBuff, pNtHdr);

    if(!Relocate(this->pBuff, pNtHdr, (FIELD_PTR)this->pBuff)){
        std::cout << "$ Error applying relocations." << std::endl;
        goto __ERROR;
    }

    printf("$ Successfully applied relocations. \n");

    if(!LoadImports(pBuff, pNtHdr)){
        printf("$ Error loading imports. \n");
        goto __ERROR;
    }

    this->lpPath = strdup(args[1]);
    this->dwSizeOfImage = pNtHdr->OptionalHeader.SizeOfImage;

    std::cout << "\n\n$ Successfully mapped PE into memory." << std::endl;
    return;

__ERROR:
    if(this->pRawPe)
        HeapFree(GetProcessHeap(), 0, this->pRawPe);
    
    if(this->pBuff)
        VirtualFree(this->pBuff, pNtHdr->OptionalHeader.SizeOfImage, MEM_RELEASE);
}