#include <stdio.h>
#include <sys/stat.h> //getcwd() mkdir()
#include <string.h> //strcmp()
//#include <stdlib.h> //malloc()
#include <errno.h> //errno = 0, perror(), strerror(), ENOENT, EEXIST
//Whenever you want to read errno to see if there are any errors, initialize it to 0 beforehend.(cuz if there is no error, errno will not be change from it's last state.
//#include <unistd.h>
//Do not forget to put '\0' at the end of every string

#define maxStrInALine 10 //Assuming each line has a maximum of 10 strings and each string has less than 400 characters
#define maxCharInAStr 400
#define maxDepth 2*maxStrInALine
#define maxFileNameLen maxCharInAStr/10
char cwd[maxCharInAStr], lineBuff[maxStrInALine][maxCharInAStr]; //sizeof(cwd) = 400, cwd = C:\Users\John\Desktop\Folder, linebuff = {"createfile", "--file", "/root/t.e st/my.txt"}
char directory[maxDepth][maxFileNameLen]; //For example {"C:", "Users", "John"}

char whitespace(){ //ignore the spaces
    char c = ' ';
    while(c == ' ') c = getchar();
    return c;
}
int getline(char out[maxStrInALine][maxCharInAStr]){ //returns the number of strings(words) in the line
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
void display(char *message){
    if(message[0] == '\0'){
        printf("Error.\n");
    }
    else{
        printf("%s\n", message);
    }
}
void Earg(int n, int mode){
    if(mode == -1) printf("This command needs atleast %d arguments.\n", n);
    else if(mode == 0) printf("This command needs exactly %d arguments.\n", n);
    else if(mode == 1) printf("This command needs less than %d arguments.\n", n);
}
int parseDir(char *in, char directory[maxDepth][maxFileNameLen]){ // must be in: '/root/dir1/dir2/file.txt' or '/root' or '/root/' format. This function manipulates the directory variable.
    //'in' mustnot be empty.
    int i = 0, Depth = -1, FNM = 0;
    while(in[i] != '\0'){
        if(in[i] == '/'){
            if(Depth != -1) directory[Depth][FNM + 1] = '\0';
            i++;
            Depth++;
            FNM = 0;
        }
        else{
            directory[Depth][FNM] = in[i];
            i++;
            FNM++;
        }
    }

    directory[Depth][FNM + 1] = '\0'; //Previous bug. Forgot to put the '\0' at the end. Very dangerous.
    if(in[i-1] == '/') Depth--;
    return Depth+1;
}

int createDir(char directory[maxDepth][maxFileNameLen], int depth){ //This function manipulates the cwd variable.
    getcwd(cwd, sizeof(cwd));
    //printf("%s, %d\n", cwd, sizeof(cwd)); for some reason if pass cwd into the function instead of calling it globally, this sizeof(cwd) would be 8
    for(int i = 0; i < depth-1; i++){
        strcat(cwd, "/");
        strcat(cwd, directory[i]);
        //printf("%s \n", cwd);
        if( (mkdir(cwd)) == -1){
            //EEXIST: pathname already exists (not necessarily as a directory). This includes the case where pathname is a symbolic link, dangling or not.
            if(errno != EEXIST) return -2; //Unknown error
        }
    }

    strcat(cwd, "/");
    strcat(cwd, directory[depth-1]);
    int isFile=0;
    while(directory[depth-1][isFile] != '\0'){
        if(directory[depth-1][isFile] == '.'){
            isFile = -1;
            break;
        }
        isFile++;
    }
    if(isFile == -1){
        FILE *file;
        if ((file = fopen(cwd, "r")))
        {
            fclose(file);
            return -1; //File already exists
        }
        file = fopen(cwd, "w");
        fclose(file);
        return 0; //File created successfully
    }
    else{
        if( (mkdir(cwd)) == -1){
            if(errno != EEXIST) return -2; //Unknown error
        }
        return 1; //directory created successfully.
    }
}

int main(){
    //Create Root
    getcwd(cwd, sizeof(cwd)); //can handle dot?
    //printf("%s, %d\n", cwd, sizeof(cwd));
    strcat(cwd, "/root"); // Beware of \ vs \\ . l
    mkdir(cwd); //returns 0 if successful and -1 otherwise

    while(1){
        int nos = getline(lineBuff); //Number of strings(words)
        if(nos == 0) continue;

        else if(!strcmp(lineBuff[0], "createfile")){
            if(nos != 3){
                //Earg(2, 0);
                display("Error: The format should be 'createfile --file <file name and address>'");
                continue;
            }
            else if(!strcmp(lineBuff[1], "--file")){
                int temp = parseDir(lineBuff[2], directory);
                /*
                for(int i = 0; i < n; i++){
                    printf("%s\n", directory[i]);
                }*/
                temp = createDir(directory, temp);
                if(temp == 0) display("File created successfully.");
                else if(temp == -1) display("Error: The file already exists.");
                else if(temp == 1) display("Directory created successfully.");
                else if(temp == -2) display("An unknown error occured.");
                else display("What?");
            }
            else{
                display("Error: The format should be 'createfile --file <file name and address>'");
            }
        }

        else{
            display("invalid command");
        }
    }
}






