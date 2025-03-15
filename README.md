```CPP
                      
                     ________  ___       ________  ___  ___  ________      
                    |\   __  \|\  \     |\   __  \|\  \|\  \|\   ____\     
                    \ \  \|\  \ \  \    \ \  \|\ /\ \  \\\  \ \  \___|_    
                     \ \   __  \ \  \    \ \   __  \ \  \\\  \ \_____  \   
                      \ \  \ \  \ \  \____\ \  \|\  \ \  \\\  \|____|\  \  
                       \ \__\ \__\ \_______\ \_______\ \_______\____\_\  \ 
                        \|__|\|__|\|_______|\|_______|\|_______|\_________\
                                                               \|_________|
                                                                         
                                ---A cli PE viewer in x32 & x64----

```

**Albus** is a base PE viewer in CLI developped to apply POO in CPP & due to my interest for Windows development aiming to create a simple tool to manipulate PE files easily.


>[!Important]
>This repo contains samples I wroted. It may not be perfect so don't blame me if you see potentials errors. => don't hesitate to reach me on discord

## You'll find : 

🟢 **Albus cli** : Full operationnal CLI with builtins commands.

🟢 **PE mapper** : Manually map PE into memory (local process virtual memory).

🟢 **PE lib** : Functions to play with PE files (relocations, load imports, map sections & other)

🟢 **PE infos** : Display all interesting informations about the PE.

🔴 **PE section DUMP** : DUMP PE section to view it.

🔴 **PE add section** : Add a new section to dedicated PE file.

🔴 **PE perms update** : Updating permissions for a dedicated section

🔴 **IN PROGRESS...**

---

## BUILTINS : 

`pwd` : current path to working directory

`exit` : exit & unload pe

`help` : display helping informations

`load <path_to_pe>` : load a pe into memory

`unload` : unload PE from memory

`infos` : display informations about the current loaded PE file.

## Samples :

....
