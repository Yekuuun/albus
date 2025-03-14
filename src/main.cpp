/**
 * Author : Yekuuun
 * Github : https://github.com/Yekuuun
 * 
 * Notes  : base implemention of a PE reversing tool using CLI.
 */
#include "cmd.hpp"

//entry point.
int main(int argc, char **argv){
    Cmd cmd;
    cmd.RunShell();
    
    return EXIT_SUCCESS;
}