#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h>

char cwd[1000], tempstr[1000]; //sizeof(cwd) = 1000 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
int eol; //end of line

int getstr(char *out){ //Ignores the beginning spaces. handles "str". puts a '\0' at the end of *out. Assumes sizeof(out) is enough. eliminates 1 whitespace after the word in the input. does not write '\n'.
    char c = ' ';
    while(c == ' ') c = getchar();
    int i = 0;
    if(c == '"'){
        while((c=getchar()) != '"'){ //I'm expecting it to end with a "
            out[i] = c;
            i++;
            if(c == '\n'){
                out[i] = '\0';
                return -1; //error
            }
        }
        c = getchar();
        if((c != ' ') && (c != '\n')){
            out[i] = '\0';
            return -1; //error
        }
    }
    else if(c == '\n'){

    }
    else{
        while((c != ' ') && (c != '\n')){
            out[i] = c;
            i++;
            c = getchar();
        }
    }
    out[i] = '\0';
    if(c == '\n') return 1;
    return 0; //Success
    //returns 0 if the word ends with a space, 1 if it ends with a '\n' and -1 if an error occurs.
}

void error(char *message){
    if(message[0] == '\0'){
        printf("Error.\n");
    }
    else{
        printf("%s\n", message);
    }
}

int main(){
    //Create Root
    getcwd(cwd, sizeof(cwd)); //can handle dot?
    strcat(cwd, "\\root"); // Beware of \ vs \\ . l
    mkdir(cwd); //returns 0 if successful and -1 otherwise

    //char com[50]; //command aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    while(1){
        eol = getstr(tempstr);
        if(!strcmp(tempstr, "createfile")){ //returns 0 if the two strings are the same
            if(eol == 1){
                error("'createfile' requires a '--file' flag.");
            }
            else{
                eol = getstr(tempstr);
                if(!strcmp(tempstr, "--file")){
                    eol = getstr(tempstr);

                }
                else if(tempstr[0] == '\0'){
                    error("'createfile' requires a '--file' flag."); //flag-option-attribute
                }
                else{
                    error("Not recognized. 'createfile' requires a '--file' flag.");
                    while(eol != 1) eol = getstr(tempstr);
                }
            }
        }


    }
}






