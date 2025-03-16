/**
 * Author : Yekuuun
 * Github : https://github.com/Yekuuun
 * 
 * This class implements the base business logic for manipulating PE from MEMORY.
 */

#include "pe.hpp"
#include "ft_console.hpp"

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
    this->AddBuiltin("dump", &Pe::Dump);
    this->AddBuiltin("parse", &Pe::Parse);
}

/**
 * Clean memory for exit.
 */
VOID Pe::Clean(){
    this->dwSizeOfImage = 0;

    if (this->pBuff) {
        MEMORY_BASIC_INFORMATION mbi;

        if (VirtualQuery(this->pBuff, &mbi, sizeof(mbi)))
            VirtualFree(this->pBuff, 0, MEM_RELEASE);

        this->pBuff = nullptr;
    }

    if(this->pRawPe){
        HeapFree(GetProcessHeap(), 0, this->pRawPe);
        this->pRawPe = nullptr;
    }

    if(this->lpPath){
        free((VOID*)this->lpPath);
        this->lpPath = nullptr;
    }
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
        std::cout << "\n================ LOADED FILE ================" << std::endl;

        std::cout << "Name :" << this->lpPath << std::endl;
        std::cout << "Image size : " << this->dwSizeOfImage << " bytes" << std::endl;
        std::cout << "Base address : " << "0x" << static_cast<void*>(this->pBuff) << std::endl;

        std::cout << "==============================================" << std::endl;
        Ft_console::SetBashColor(7);
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
        this->Clean();
        std::cout << "$ Successfully unloaded PE file." << std::endl;
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
    
    std::cout << "\n Loading PE into memory : \n" << std::endl;
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

    std::cout << "\n\n$ Successfully mapped PE into memory.\n" << std::endl;
    return;

__ERROR:
    this->Clean();
}

/**
 * Dump a section from PE FILE.
 */
VOID Pe::Dump(IN CHAR **args){
    if(args[1] == nullptr){
        std::cout << "$ dump command must get a single param <section_name>. Use help command to get more informations." << std::endl;
        return;
    }

    if(this->pBuff == nullptr){
        std::cout << "$ No loaded PE file. Use load command to load file into memory." << std::endl;
        return;
    }

    CHAR *cSection = args[1];
    SIZE_T sSection = strlen(cSection);

    if(sSection > 8){
        std::cout << "$ section name must be max 8 characters. \n" << std::endl;
        return; //max section name => 8
    }

    PIMAGE_NT_HEADERS pNtHdr = GetNtHdr(this->pBuff);
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHdr);

    if(pSection == nullptr)
        return;

    for(WORD i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++){
        BYTE* pSectionName = (BYTE*)pSection[i].Name;
        if(memcmp(pSectionName, cSection, sSection) == 0){
            std::cout << "$ Found " << cSection << " section" << std::endl;
            printf("\t- Virtual Size: 0x%08X\n", pSection[i].Misc.VirtualSize);
            printf("\t- Raw Size: 0x%08X\n", pSection[i].SizeOfRawData);

            std::cout << "\n================DUMPING SECTION================" << std::endl;

            BYTE* pStartSection = ((BYTE*)pRawPe + pSection[i].PointerToRawData);
            for(DWORD j = 0; j < pSection[i].SizeOfRawData; j++){
                if(j % 16 == 0) printf("\n%04X: ", j);
                printf("%02X ", pStartSection[j]);
            }

            printf("\n");
            std::cout << "==================================================" << std::endl;
            return;
        }
    }

    std::cout << "$ " << cSection << " not found..." << std::endl;
}

/**
 * Parsing PE file.
 */
VOID Pe::Parse(IN CHAR**){
    if(this->pBuff == nullptr){
        std::cout << "$ no loaded PE in memory. use load command, try help command for more informations." << std::endl;
        return;
    }

    LPVOID pBaseAddress = (LPVOID)this->pBuff;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cout << "$ Invalid PE signature (MZ missing)." << std::endl;
        return;
    }

    // Récupérer l'en-tête NT
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pBaseAddress + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cout << "$ Invalid PE signature (PE missing)." << std::endl;
        return;
    }

    PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeaders->OptionalHeader;

    std::cout << "\n================ PE HEADER INFO ================\n";
    std::cout << "$ Entry Point Address : 0x" << std::hex << pOptionalHeader->AddressOfEntryPoint << std::endl;
    std::cout << "$ Image Base Address  : 0x" << std::hex << pOptionalHeader->ImageBase << std::endl;
    std::cout << "$ Size of Image       : 0x" << std::hex << pOptionalHeader->SizeOfImage << " bytes" << std::endl;
    std::cout << "$ Number of Sections  : " << std::dec << pNtHeaders->FileHeader.NumberOfSections << std::endl;

    PIMAGE_DATA_DIRECTORY pImportDirectory = &pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (pImportDirectory->VirtualAddress != 0) {
        std::cout << "$ Import Table Address : 0x" << std::hex << pImportDirectory->VirtualAddress << std::endl;
    } 
    else {
        std::cout << "$ No Import Table found." << std::endl;
    }

    PIMAGE_DATA_DIRECTORY pRelocDirectory = &pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (pRelocDirectory->VirtualAddress != 0) {
        std::cout << "$ Relocation Table Address : 0x" << std::hex << pRelocDirectory->VirtualAddress << std::endl;
    } 
    else {
        std::cout << "$ No Relocation Table found." << std::endl;
    }

    PIMAGE_DATA_DIRECTORY pExportDirectory = &pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (pExportDirectory->VirtualAddress != 0) {
        std::cout << "$ Export Table Address : 0x" << std::hex << pExportDirectory->VirtualAddress << std::endl;
    } 
    else {
        std::cout << "$ No Export Table found." << std::endl;
    }

    std::cout << "================================================\n" << std::endl;
}