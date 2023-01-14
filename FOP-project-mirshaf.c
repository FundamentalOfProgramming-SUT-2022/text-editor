#include <stdio.h>
#include <sys/stat.h> //getcwd() mkdir()
#include <string.h> //strcmp()
#include <stdlib.h> //malloc()
//#include <unistd.h>

#define maxStrInALine 10 //Assuming each line has a maximum of 10 strings and each string has less than 400 characters
#define maxCharInAStr 400
char cwd[maxCharInAStr], lineBuff[maxStrInALine][maxCharInAStr]; //sizeof(cwd) = 400

char whitespace(){ //ignore the spaces
    char c = ' ';
    while(c == ' ') c = getchar();
    return c;
}
int getline(char out[maxStrInALine][maxCharInAStr]){ //returns the number of strings in the line
    char c;
    int currentStr = 0, currentChar = 0;
    while( (c = whitespace()) != '\n'){
        currentChar = 0;
        if(c == '"'){
            while((c = getchar()) != '"'){
                if(c == '\n') return -1;
                out[currentStr][currentChar] = c;
                currentChar++;
            }
            out[currentStr][currentChar] = '\0';
        }
        else{
            do{
                if(c == '\n'){
                    out[currentStr][currentChar] = '\0';
                    return currentStr+1;
                }
                out[currentStr][currentChar] = c;
                currentChar++;
            }while((c = getchar()) != ' ');

            out[currentStr][currentChar] = '\0';
        }
        currentStr++;
    }
    return currentStr;
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
        int nos = getline(lineBuff); //Number of strings(words)
        for(int i = 0; i < nos; i++){
            printf("'%s' ", lineBuff[i]);
        }
        printf("\n");
    }
}






