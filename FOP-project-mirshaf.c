#include <stdio.h>
#include <sys/stat.h> //getcwd(), mkdir(), struct stat, stat(), S_ISDIR, things used in recursivetree(), ...
#include <string.h> //strcmp()
#include <stdlib.h> //atoi()
//#include <unistd.h> //existDir
#include <errno.h> //errno = 0, perror(), strerror(), ENOENT, EEXIST
#include <dirent.h> //struct dirent, DIR, readdir(), things used in recursivetree(), ...

/*Notes:
This program is written for windows.
Whenever you want to read errno to see if there are any errors, initialize it to 0 beforehend.(cuz if there is no error, errno will not be change from it's last state.
Do not forget to put '\0' at the end of every string
Remember to close every file when you are done with it.
The inputs are guaranteed to be in the specified structures. And in order.

"root/" and "root" are acceptable but "/root" is not! You must handle both inputs though.
the input is like "/root/my.txt" instead of "root/my.txt" so I will +1 the directoryStr pointer.
IN SHORT: there should not be a '/' at the start or the end of the directory. "/root/me.txt/" is not acceptable(vs "/root/me.txt") but should be handled.
currently if there is a "--pos" in the input, there must follow a "%d:%d" even if in an invalid command.
if the user writes a command inside of a --str attribute for example, something might go wrong.
find() and replace() may act weird for *Hi in "HiHi".
if --pos is out of bounds then cut() will display 2 errors instead of 1.
if two files share a name then undo() will break. To fix this you can create a full directory identical to root and backup everything there.
BRUH somethings wrong with createdir --file /root/dir/file.txt
if the two commands next to arman(=D) share flags, something might go wrong
arman might mess with the nos variable

*/

#define maxStrInALine 25 //Assuming each line has a maximum of 10 strings and each string has less than 400 characters
#define maxCharInAStr 450
#define maxDepth 2*maxStrInALine
#define maxFileNameLen maxCharInAStr/10
#define FILEBUFFER "fileBuff.txt"

#define OUTPUTBUFFER "outputBuffer.txt"
FILE *output_buffer;

char cwd[maxCharInAStr], lineBuff[maxStrInALine][maxCharInAStr]; //sizeof(cwd) = 400, cwd = C:\Users\John\Desktop\Folder, linebuff = {"createfile", "--file", "/root/t.e st/my.txt"}
char directory[maxDepth][maxFileNameLen]; //For example {"C:", "Users", "John"}

void display(char *message){
    if(message[0] == '\0'){
        fprintf(output_buffer, "Error.\n");
    }
    else{
        fprintf(output_buffer, "%s\n", message);
    }
}

int existDir(const char * const path){ //can get either absolute or relative path
    struct stat stats;
    int isFile = 0;
    char temp[maxCharInAStr];
    strcpy(temp, path);
    for(int i = strlen(temp) - 1; i >= 0; i--){
        if(temp[i] == '.') isFile = 1;
        if( (temp[i] == '/') && i!=(strlen(temp) - 1) ){
            if(isFile){
                temp[i] = '\0';
            }
            break;
        }
    }

    stat(temp, &stats); //save information about the file pointed to by "path" to "stats"

    // Check for file existence
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}
int existFile(const char * const fname){ //can get either absolute or relative path
    FILE *file;

    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}
int handleExistence(char *path){ //Can change the path section of "lineBuff". Warning: Currently IS NECESSARY FOR CORRECTING THE PATH.
    //Warning: the following might cause memory leak.
    if(path[0] == '/'){
        char temp[maxCharInAStr];
        strcpy(temp, path+1);
        strcpy(path, temp);
    } // "root/me.txt" is acceptable but "/root/me.txt" is not!
    if(path[strlen(path) - 1] == '/') path[strlen(path) - 1] = '\0'; // "root/me.txt" is acceptable but "root/me.txt/" is not!

    if(existDir(path)){
        if(existFile(path)){
            return 1;
        }
        display("There is no such file in that directory.");
        return 0;
    }
    display("There is no such directory.");
    return 0;
}

char whitespace(){ //ignore the spaces
    char c = ' ';
    while(c == ' ') c = getchar();
    return c;
}
int getline(char out[maxStrInALine][maxCharInAStr]){ //returns the number of strings(words) in the line
    char c;
    int currentStr = 0, currentChar = 0, EFlag = 0; //Error Flag
    while( (c = whitespace()) != '\n'){ //take input until \n
        currentChar = 0;
        if( (c == '"') && (1) ){ //the first '"' is guaranteed to come after a space.
            //The following can cause an erroooooooooooooooooooooooooor because of [currentchar-1]
            while( ((c = getchar()) != '"') || ( (out[currentStr][currentChar - 1] == '\\') && (out[currentStr][currentChar - 2] != '\\')) ){ //can handle "he said \"Hi!\""
                if(c == '\n') return -1; //Attentioooooooooooooooooooooooooooooon. should return -2.
                out[currentStr][currentChar] = c;
                currentChar++;
            }
            out[currentStr][currentChar] = '\0';
        }
        else{
            do{
                if(c == '\n'){
                    out[currentStr][currentChar] = '\0';
                    if(EFlag) return -1;
                    return currentStr+1; //Attentioooooooooooooooooooooooooooooon
                }
                out[currentStr][currentChar] = c;
                currentChar++;
            }while((c = getchar()) != ' ');

            out[currentStr][currentChar] = '\0';
        }
        currentStr++;
        if(currentStr >= maxStrInALine - 1){
            currentStr = 1;
            EFlag = 1;
        }

        if(!strcmp(out[currentStr-1], "--pos")){
            currentChar = 0;
            while((c = whitespace()) != ':'){
                out[currentStr][currentChar] = c;
                currentChar++;
            }
            out[currentStr][currentChar] = '\0';
            currentStr++;
            currentChar = 0;
            while((c = getchar()) != ' '){
                if(c == '\n'){
                    out[currentStr][currentChar] = '\0';
                    if(EFlag) return -1;
                    return currentStr+1; //Attentioooooooooooooooooooooooooooooon
                }
                out[currentStr][currentChar] = c;
                currentChar++;
            }
            out[currentStr][currentChar] = '\0';
            currentStr++;
        }
    }
    if(EFlag) return -1; //if there is too many words in one line
    return currentStr; //The function usually has already returned above in the loop
}
int processLine(char lineBuff[maxStrInALine][maxCharInAStr], int nos){ //number of strings
    //Add an indicator of the end:
    strcpy(lineBuff[nos], "EOL"); //End Of Line

    //Handle backslash characters:
    for(int i = 0; i < nos; i++){
        char c = '\0';
        for(int j = 0; (c = lineBuff[i][j]) != '\0'; j++){
            if(c == '\\' ){
                char temp = lineBuff[i][j+1], toBe = '\0';
                if(temp == 'n') toBe = '\n';
                else if(temp == '\\') toBe = '\\';
                else if(temp == 't') toBe = '\t';
                else if(temp == '"') toBe = '"';

                if(toBe != '\0'){
                    char temp[maxCharInAStr];
                    for(int k = 0; k < j; k++){
                        temp[k] = lineBuff[i][k];
                    }
                    temp[j] = toBe;
                    for(int k = j+2; 1; k++){
                        if(lineBuff[i][k] == '\0'){
                            temp[k-1] = '\0'; //I once again forgot to put '\0' and encountered a bug
                            break;
                        }
                        temp[k-1] = lineBuff[i][k];
                    }
                    strcpy(lineBuff[i], temp);
                    //i--;
                    //break;
                    //instead:
                    continue;
                }
            }
        }
    }

    /*BUG!
    //Correct the path:
    for(int i = 0; i < nos-1; i++){
        if(!strcmp(lineBuff[i], "--file")){
            //Warning: the following might cause memory leak.
            if(lineBuff[i+1][0] == '/'){
                char temp[maxCharInAStr];
                strcpy(temp, lineBuff[i+1]+1);
                strcpy(lineBuff[i+1], temp);
            } // "root/me.txt" is acceptable but "/root/me.txt" is not!
            if(lineBuff[i+1][strlen(lineBuff[i+1]) - 1] == '/') lineBuff[i+1][strlen(lineBuff[i+1]) - 1] = '\0'; // "root/me.txt" is acceptable but "root/me.txt/" is not!
        }
    }*/
}

int parseDir(const char *in, char directory[maxDepth][maxFileNameLen]){ // must be in: '/root/dir1/dir2/file.txt' or '/root' or '/root/' format. This function manipulates the directory variable.
    //'in' mustnot be empty.
    //returns the number of separated strings
    int i = 0, Depth = -1, FNM = 0;
    while(in[i] != '\0'){
        if(in[i] == '/'){ //Beware that we are not handling backslash
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
int createDir(const char directory[maxDepth][maxFileNameLen], int depth){ //This function manipulates the cwd variable.
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
            else return -1; //File(should be directory) already exists
        }
        return 1; //directory created successfully.
    }
}

//Helper functions:
int copyFile(const char source_file[maxCharInAStr], const char dest_file[maxCharInAStr]){ //If dest_file exists, it will be overwritten. source_file must exist.
    FILE *filedst = fopen(dest_file, "w"), *filesrc = fopen(source_file, "r");
    char c;
    while(1){
        c = fgetc(filesrc);
        if(feof(filesrc)) break;
        fputc(c, filedst);
    }
    fclose(filedst);
    fclose(filesrc);
    return 0;
}
int createBackup(const char *fileDir, char directory[maxDepth][maxFileNameLen]){
    getcwd(cwd, sizeof(cwd));
    int temp = parseDir(fileDir, directory);
    /*FILE *fromFile = fopen(fileDir, "r"), *toFile = fopen(directory[temp-1], "w");

    char c;
    while(1){
        c = fgetc(fromFile);
        if(feof(fromFile)) break;
        fputc(c, toFile);
    }

    fclose(fromFile);
    fclose(toFile);*/
    copyFile(fileDir, directory[temp-1]);
    return 0;
}
int revertFile(const char *fileDir, char directory[maxDepth][maxFileNameLen]){ //Assumes fileDir exists and is a file.
    getcwd(cwd, sizeof(cwd));
    int temp = parseDir(fileDir, directory);
    FILE *backupFile = fopen(directory[temp-1], "r");
    if(backupFile == NULL){
        fclose(backupFile);
        return -1; //There is no backup.
    }
    fclose(backupFile);

    copyFile(fileDir, FILEBUFFER); //mainFile to fileBuff
    copyFile(directory[temp-1], fileDir); //Backup to mainFile
    copyFile(FILEBUFFER, directory[temp-1]); //fileBuff to backup

    return 0;
}


int cat(){ //Uses the "lineBuff".
    FILE *mainFile = fopen(lineBuff[2], "r");
    char c;
    display("---------------------------");
    while((c = fgetc(mainFile)) != EOF){
        //if(feof(mainFile)) break;
        fprintf(output_buffer, "%c", c);
    }
    display("\n---------------------------");
    fclose(mainFile);
    return 0;
}

int fileHelper(int targetLine, int targetChar, const char file_name[maxCharInAStr]){ //Fills "fileBuff.txt" and returns something good
    FILE *mainFile = fopen(file_name, "r");
    //if(mainFile == NULL) printf("GG\n");
    //printf("%s\n", lineBuff[2]);
    FILE *fileBuff = fopen("fileBuff.txt", "w");
    int currentLine = 1, currentChar = 0;
    int targetTotalChars = 0, f = 1;
    char c;
    while((c = fgetc(mainFile)) != EOF){ // while(!feof(mainFile))
        fputc(c, fileBuff);

        if(currentLine == targetLine){
            if(currentChar == targetChar) f = 0;
            if(c!='\n') currentChar++;
        }
        if(f) targetTotalChars++;
        if(c=='\n') currentLine++;
    }
    fclose(mainFile);
    fclose(fileBuff);

    if((currentLine >= targetLine) && (targetLine>=1)){
        if((currentChar >= targetChar) && (targetChar>=0)){
            return targetTotalChars;
        }
    }
    display("Error: The specified position does not exist in the file. Remember that line_number is 1-based and char_number is 0-based.");
    return -1;
}
int insertstr(const char file_name[maxCharInAStr], const char starting_line[maxCharInAStr], const char starting_char[maxCharInAStr], const char the_str[maxCharInAStr]){
    int targetLine = atoi(starting_line), targetChar = atoi(starting_char);
    int helper = fileHelper(targetLine, targetChar, file_name);
    if(helper >= 0){
        createBackup(file_name, directory); //For the Undo command

        FILE *mainFile = fopen(file_name, "w"), *fileBuff = fopen(FILEBUFFER, "r");
        int currentLine = 1, currentChar = 0;
        char c;

        while(!feof(fileBuff)){
            if(currentLine == targetLine){
                if(currentChar == targetChar){
                    for(int i = 0; i < strlen(the_str); i++){
                        fputc(the_str[i], mainFile);
                    }
                }
                currentChar++;
            }

            if( (c = fgetc(fileBuff)) != EOF){
                fputc(c, mainFile);
            }
            if(c=='\n') currentLine++;
        }

        fclose(mainFile);
        fclose(fileBuff);
        display("Inserted successfully.");
        return 0;
    }
    return -1;
}
int removestr(){ //Uses the "lineBuff". //checks for --pos to be in range, but if -size is not in range it simply goes to the end/beginning of the file
    int targetLine = atoi(lineBuff[4]), targetChar = atoi(lineBuff[5]);
    int helper = fileHelper(targetLine, targetChar, lineBuff[2]);
    if(helper >= 0){
        createBackup(lineBuff[2], directory); //For the Undo command

        FILE *mainFile = fopen(lineBuff[2], "w"), *fileBuff = fopen("fileBuff.txt", "r");
        int f = 0, size = atoi(lineBuff[7]), totalChars = 0, targetTotalChars = helper;
        if(!strcmp(lineBuff[8], "-b")) f=1;
        char c;

        while(!feof(fileBuff)){
            if( (c = fgetc(fileBuff)) != EOF){
                if(f){ //remove backwards
                    if((totalChars < targetTotalChars-size) || (totalChars >= targetTotalChars)){
                        fputc(c, mainFile);
                    }
                }
                else{ //remove forwards
                    if((totalChars < targetTotalChars) || (totalChars >= targetTotalChars+size)){
                        fputc(c, mainFile);
                    }
                }
            }
            totalChars++;
        }

        fclose(mainFile);
        fclose(fileBuff);

        return 0;
    }
    return -1;
}
int copystr(){ //Uses the "lineBuff".
    int targetLine = atoi(lineBuff[4]), targetChar = atoi(lineBuff[5]);
    int helper = fileHelper(targetLine, targetChar, lineBuff[2]);
    if(helper >= 0){
        FILE *mainFile = fopen("clipboard.txt", "w"), *fileBuff = fopen("fileBuff.txt", "r");
        int f = 0, size = atoi(lineBuff[7]), totalChars = 0, targetTotalChars = helper;
        if(!strcmp(lineBuff[8], "-b")) f=1;
        char c;

        while(!feof(fileBuff)){
            if( (c = fgetc(fileBuff)) != EOF){
                if(f){ //remove backwards
                    if((totalChars >= targetTotalChars-size) && (totalChars < targetTotalChars)){
                        fputc(c, mainFile);
                    }
                }
                else{ //remove forwards
                    if((totalChars >= targetTotalChars) && (totalChars < targetTotalChars+size)){
                        fputc(c, mainFile);
                    }
                }
            }
            totalChars++;
            if(totalChars > targetTotalChars+size+1) break;
        }

        fclose(mainFile);
        fclose(fileBuff);
        return 0;
    }
    return -1;
}
int cutstr(){
    int a = copystr();
    int b = removestr();
    if((!a) && (!b)){
        return 0;
    }
    return -1;
}
int pastestr(){
    int targetLine = atoi(lineBuff[4]), targetChar = atoi(lineBuff[5]);
    int helper = fileHelper(targetLine, targetChar, lineBuff[2]);
    if(helper >= 0){
        FILE *clipboardFile = fopen("clipboard.txt", "r");
        if(clipboardFile == NULL){
            display("Error: The clipboard is empty.");
            return -1;
        }

        createBackup(lineBuff[2], directory); //For the Undo command

        FILE *mainFile = fopen(lineBuff[2], "w"), *fileBuff = fopen("fileBuff.txt", "r");
        int currentLine = 1, currentChar = 0;
        char c;

        while(!feof(fileBuff)){
            if(currentLine == targetLine){
                if(currentChar == targetChar){
                    while((c = fgetc(clipboardFile)) != EOF){
                        fputc(c, mainFile);
                    }
                }
                currentChar++;
            }

            if( (c = fgetc(fileBuff)) != EOF){
                fputc(c, mainFile);
            }
            if(c=='\n') currentLine++;
        }

        fclose(mainFile);
        fclose(fileBuff);
        display("Pasted successfully.");
        return 0;
    }
    return -1;
}

int FINDByword(int charNum, const char file_name[maxCharInAStr]){ //zero based. words are separated by space or \n or \t
    FILE *mainFile = fopen(file_name, "r");
    int counter = 0, wordCounter = 0;
    char c, temp = ' ';

    while((c = fgetc(mainFile)) != EOF){
        if( ((c == ' ') || (c == '\n') || (c == '\t')) && (temp != ' ') && (temp != '\n') && (temp != '\t')) wordCounter++;
        counter++;
        if(counter >= charNum) break;
        temp = c;
    }

    fclose(mainFile);
    return wordCounter;
}
int FINDHandleBackslash(char *str){
    int counter = 0;
    for(int i = 0; i < strlen(str); i++){
        if(str[i] != '*'){
            str[counter] = str[i];
            counter++;
        }
        else if(i>=1 && (str[i-1] == '\\')){
            str[counter-1] = str[i];
        }
    }
    str[counter] = '\0';
    return 0;
}
int simpleFIND(int from, const char file_name[maxCharInAStr], const char the_constant_str[maxCharInAStr], int pos[2]){ //cannot handle a wildcard in the middle of str.
    //pos[0] is where the found string begins and pos[1] is where it ends. it is to be filled by this function.
    //cannot find "app apple" in "app app apple apple" because it does not overlap.
    //MUST NOT CHANGE lineBuffer
    char the_str[maxCharInAStr];
    strcpy(the_str, the_constant_str); //Used this trick to be able to declare the_constant_str as a const

    FILE *mainFile = fopen(file_name, "r");
    char c, *str = the_str;
    int counter = 0, similarity = 0;

    if( (str[strlen(str)-1] == '*') && ((strlen(str)<=1) || (str[strlen(str)-2] != '\\')) ){ //if the str ends with a wildcard (not a "\*")
        FINDHandleBackslash(str); //Changes 'str' but not lineBuff[2]
        int flag = 0;
        while(1){
            if(similarity == strlen(str)){
                flag = 1;
                /*if((c = fgetc(mainFile)) != EOF){
                    if(c != ' '){
                        return counter-similarity;
                    }
                }*/
                pos[0] = counter - similarity;
                while((c = fgetc(mainFile)) != EOF){
                    if((c == ' ') || (c == '\n') || (c == '\t')) break;
                    counter++;
                }
                pos[1] = counter - 1;
                fclose(mainFile);
                return pos[0]; //comment this line and uncomment the above if the wildcard can't be empty.
                similarity = 0;
            }

            if(!flag) c = fgetc(mainFile);
            flag = 0;

            if((counter>=from) && (str[similarity] == c)){
                similarity++;
            }
            else{
                similarity = 0;
            }
            counter++;

            if(c == EOF) break;
        }
        return -1;
    }
    else if(str[0] == '*'){ //if the str begins with a wildcard (not a "\*")
        FINDHandleBackslash(str); //Changes 'str' but not lineBuff[2]
        int lastSpace = 0;
        similarity = 0;
        char temp;
        while(1){
            c = fgetc(mainFile);

            if(similarity == strlen(str)){
                /*if((counter-similarity == 0) || (temp != ' ')){
                    //return counter - similarity;
                    return lastSpace;
                }*/
                pos[0] = lastSpace;
                pos[1] = counter - 1;
                fclose(mainFile);
                return pos[0]; //comment this line and uncomment the above if the wildcard can't be empty.
                similarity = 0;
            }

            if((counter>=from) && (str[similarity] == c)){
                similarity++;
                counter++;
            }
            else{
                temp = c;
                counter++;
                //counter += similarity;
                similarity = 0;
            }

            if(c == ' ') lastSpace = counter;
            if(c == EOF) break;
        }
        return -1;
    }
    else{ //no wildcard
        FINDHandleBackslash(str);
        while(1){
            c = fgetc(mainFile);

            if(similarity == strlen(str)){ //Previous bug: this if statement should be checked even if c==EOF.
                pos[0] = counter - similarity;
                pos[1] = counter - 1;
                fclose(mainFile);
                return pos[0];
            }
            if((counter>=from) && (str[similarity] == c)){
                similarity++;
            }
            else{
                similarity = 0;
            }
            counter++;

            if(c == EOF) break;
        }
        return -1;
    }

    fclose(mainFile);
}
int findInFile(int nos, const char file_name[maxCharInAStr], const char the_str[maxCharInAStr]){ //number of strings. WARNING: Still needs access to lineBuff to check for flags.
    int fCount = 0, fAt = 0, fByword = 0, fAll = 0, atWhere;
    for(int i = 0; i < nos; i++){
        if(!strcmp(lineBuff[i], "-count")) fCount = 1;
        if(!strcmp(lineBuff[i], "-at")){
            fAt = 1;
            atWhere = atoi(lineBuff[i + 1]);
        }
        if(!strcmp(lineBuff[i], "-byword")) fByword = 1;
        if(!strcmp(lineBuff[i], "-all")) fAll = 1;
    }
    if(fCount){
        if(fAt || fByword || fAll){
            display("Error: Invalid combination of flags. [-count] cannot be accompanied by another flag.");
            return 0;
        }
    }
    if(fAt){
        if(fAll){
            display("Error: Invalid combination of flags. [-at] and [-all] cannot come together.");
            return 0;
        }
    }

    int pos[2]; //This is not needed for findInFile() but needs to be passed to simpleFind().
    if(!fCount && !fAt && !fAll){
        int temp = simpleFIND(0, file_name, the_str, pos);
        if(fByword) temp = FINDByword(temp, file_name);
        fprintf(output_buffer, "<%d>\n", temp);
        return 0;
    }
    else if(fCount){
        int count = 0, from = 0, temp;
        while((temp = simpleFIND(from, file_name, the_str, pos)) != -1){
            count++;
            from = pos[1]+1; //from = temp + 1;
        }
        fprintf(output_buffer, "<%d>\n", count);
        return 0;
    }
    else if(fAt){
        int count = 0, from = 0, temp;
        while((temp = simpleFIND(from, file_name, the_str, pos)) != -1){
            count++;
            from = pos[1]+1;
            if(count == atWhere) break;
        }
        if((count < atWhere) || (temp == -1)) display("<-1>");
        else{
            if(fByword) temp = FINDByword(temp, file_name);
            fprintf(output_buffer, "<%d>\n", temp);
        }
        return 0;
    }
    else if(fAll){
        int count = 0, from = 0, temp;
        while((temp = simpleFIND(from, file_name, the_str, pos)) != -1){
            count++;
            from = from = pos[1]+1;
            if(fByword) temp = FINDByword(temp, file_name);
            fprintf(output_buffer, "<%d> ", temp);
        }
        if(count<1) fprintf(output_buffer, "<-1>");
        fprintf(output_buffer, "\n");
        return 0;
    }

    return -1;
}
int replaceInFile(int nos, const char file_name[maxCharInAStr], const char initialStr[maxCharInAStr], const char finalStr[maxCharInAStr]){ //WARNING: Still needs access to lineBuff to check for flags.
    //nos==number of strings.
    int fAt = 0, fAll = 0, atWhere = 1;
    for(int i = 0; i < nos; i++){
        if(!strcmp(lineBuff[i], "-at")){
            fAt = 1;
            atWhere = atoi(lineBuff[i + 1]);
        }
        if(!strcmp(lineBuff[i], "-all")) fAll = 1;
    }
    if(fAt && fAll){
        display("Error: Invalid combination of flags. [-at] and [-all] cannot come together.");
        return 0;
    }


    if(fAll){
        int count = 0, from = 0, temp, pos[2], BackUpFlag = 0;
        while((temp = simpleFIND(0, file_name, initialStr, pos)) != -1){
            if(BackUpFlag == 0){
                createBackup(file_name, directory); //For the Undo command
                BackUpFlag = 1;
            }

            count++;
            from = pos[1]+1;

            //Replacing process:
            copyFile(file_name, FILEBUFFER); //mainFile to fileBuff

            int tempCounter = 0;
            FILE *fileBuff = fopen(FILEBUFFER, "r"), *mainFile = fopen(file_name, "w");
            char c;
            while(1){
                if(tempCounter == temp){
                    for(int i = 0; i < strlen(finalStr); i++){
                        fputc(finalStr[i], mainFile);
                    }
                }

                c = fgetc(fileBuff);
                if(feof(fileBuff)) break;
                if((tempCounter < pos[0]) || (pos[1] < tempCounter)){ //temp is where the initialStr begins.
                    fputc(c, mainFile);
                }
                tempCounter++;
            }
            fclose(mainFile);
            fclose(fileBuff);
        }
        if(count<1){
            display("Error: The initial string was not found in the file.");
            return -1;
        }
        display("Replaced successfully.");
        return 0;
    }
    else { //if there's not -all then there must be -at
        int count = 0, from = 0, temp, pos[2];
        while((temp = simpleFIND(from, file_name, initialStr, pos)) != -1){
            count++;
            from = pos[1]+1;
            if(count == atWhere) break;
        }
        if((count < atWhere) || (temp == -1)){
            display("Error: There are not enough iterations of the initial string in the file.");
            return -1;
        }
        else{
            createBackup(file_name, directory); //For the Undo command

            //Replacing process:
            copyFile(file_name, FILEBUFFER); //mainFile to fileBuff

            int tempCounter = 0;
            FILE *fileBuff = fopen(FILEBUFFER, "r"), *mainFile = fopen(file_name, "w");
            char c;
            while(1){
                if(tempCounter == temp){
                    for(int i = 0; i < strlen(finalStr); i++){
                        fputc(finalStr[i], mainFile);
                    }
                }

                c = fgetc(fileBuff);
                if(feof(fileBuff)) break;
                if((tempCounter < pos[0]) || (pos[1] < tempCounter)){ //temp is where the initialStr begins.
                    fputc(c, mainFile);
                }
                tempCounter++;
            }
            fclose(mainFile);
            fclose(fileBuff);

            display("Replaced successfully.");
            return 0;
        }
    }
}

int myGrep(int flagL, int flagC, const char str[maxCharInAStr], const char file_name[maxCharInAStr]){ //return value is not standard.
    int pos[2];
    if(flagL){
        if(simpleFIND(0, file_name, str, pos) != -1){
            fprintf(output_buffer, "%s\n", file_name);
            return 1;
        }
        return -1;
    }
    else{
        int from = 0, charCounter = 0, ansCounter = 0, currentLine = 1, lastLine = 0, temp;
        long int firstCharInLine = 0;
        char c;
        while((temp = simpleFIND(from, file_name, str, pos)) != -1){
            //printf("AAA\n");
            FILE *mainFile = fopen(file_name, "r");

            charCounter = 0;
            currentLine = 1;

            while((c = fgetc(mainFile)) != EOF){
                //printf("<%c>", c);
                if(charCounter == temp){
                    if(currentLine > lastLine){
                        lastLine = currentLine;
                        ansCounter++;
                        if(!flagC){
                            fprintf(output_buffer, "%s: ", file_name);
                            long whereWereWe = ftell(mainFile);
                            fseek(mainFile, firstCharInLine, SEEK_SET);
                            char temporaryChar;
                            while((temporaryChar = fgetc(mainFile)) != EOF){
                                fprintf(output_buffer, "%c", temporaryChar);
                                if(temporaryChar == '\n') break;
                            }
                            if(temporaryChar != '\n') fprintf(output_buffer, "\n");
                            fseek(mainFile, whereWereWe, SEEK_SET);

                        }
                    }
                    break;
                }
                if(c == '\n'){
                    currentLine++;
                    firstCharInLine = ftell(mainFile);
                }
                charCounter++;
            }
            fclose(mainFile);
            from = pos[1];
        }
        return ansCounter;

    }
}

int indentFile(const char file_name[maxCharInAStr]){ //introduced fileBuff2
    copyFile(file_name, "fileBuff2.txt");
    FILE *mainFile = fopen(file_name, "w"), *fileBuff = fopen(FILEBUFFER, "w"), *copyOfMainFile = fopen("fileBuff2.txt", "r");
    char c, lastNonWhiteChar = ' ';
    int howDeep = 0, shouldClosingGetNL = 0;
    while((c = fgetc(copyOfMainFile)) != EOF){

        if((c == '\n') || (lastNonWhiteChar != '}')) shouldClosingGetNL = 1; //This is to fix "int main() { if (1 == 1){printf(“Hello world”);} }"
        else shouldClosingGetNL = 0;

        if((c != ' ') && (c != '\t') && (c != '\n')){

            if((lastNonWhiteChar == '}') && (c != '}')){ //This is to fix "int main() { if (1 == 1){printf(“Hello world”);} }"
            for(int i = 0; i < howDeep; i++){
                fputc('\t', mainFile);
            }
            }

            if((c != '{') && (c != '}')){
                if((lastNonWhiteChar != '{') && (lastNonWhiteChar != '}')){
                    //Write the previous whitespace:
                    fclose(fileBuff);
                    fileBuff = fopen(FILEBUFFER, "r");
                    char c2;
                    while((c2 = fgetc(fileBuff)) != EOF){
                        fputc(c2, mainFile);
                    }
                    fclose(fileBuff);
                    fileBuff = fopen(FILEBUFFER, "w");
                }
                else{
                    //Erase the previous whitespace:
                    fclose(fileBuff);
                    fileBuff = fopen(FILEBUFFER, "w");
                }

                fputc(c, mainFile);
            }
        }
        else{
            fputc(c, fileBuff);
        }

        if(c == '{'){
            if((lastNonWhiteChar != ' ') && (lastNonWhiteChar != '{')){
                fputc(' ', mainFile);

                //Erase the previous whitespace:
                fclose(fileBuff);
                fileBuff = fopen(FILEBUFFER, "w");
            }
            fputc('{', mainFile);

            howDeep++;
            fputc('\n', mainFile);
            for(int i = 0; i < howDeep; i++){
                fputc('\t', mainFile);
            }
        }
        else if(c == '}'){
            //if(lastNonWhiteChar == '{'){
            //    fputc('\n', mainFile);
            //}
            if(shouldClosingGetNL == 1) {fputc('\n', mainFile);}
            howDeep--;
            for(int i = 0; i < howDeep; i++){
                fputc('\t', mainFile);
            }
            fputc('}', mainFile);
            fputc('\n', mainFile);
        }

        if((c != ' ') && (c != '\t') && (c != '\n')){
            lastNonWhiteChar = c;
        }
    }

    fclose(mainFile);
    fclose(fileBuff);
    fclose(copyOfMainFile);
    return 0;
}

int howManyLines(const char file_name[maxCharInAStr]){
    FILE *mainFile = fopen(file_name, "r");
    int lineNum = 1;
    char c;
    while((c = fgetc(mainFile)) != EOF){
        if(c == '\n') lineNum++;
    }

    fclose(mainFile);
    return lineNum;
}
int displayLineOfFile(int lineNum, const char file_name[maxCharInAStr]){ //assumes file_name exists.
    FILE *mainFile = fopen(file_name, "r");
    int currentLine = 1;
    char c;
    while((c = fgetc(mainFile)) != EOF){
        if(c == '\n'){
            currentLine++;
            if(currentLine == (lineNum+1)){
                fprintf(output_buffer, "\n");
                fclose(mainFile);
                return 0;
            }
        }
        if(currentLine == lineNum){
            if(c != '\n') fprintf(output_buffer, "%c", c);
        }
    }

    fclose(mainFile);
    if(currentLine == lineNum){
        fprintf(output_buffer, "\n");
        return 0;
    }
    else return -1;
}
int textComparator(const char file_name1[maxCharInAStr], const char file_name2[maxCharInAStr]){
    FILE *file1 = fopen(file_name1, "r"), *file2 = fopen(file_name2, "r");
    int charNum1= 0, charNum2 = 0, lineNum1 = 1, lineNum2 = 1, eof1 = 0, eof2 = 0, theFilesAreTheSame = 1;
    char c1, c2, prevC1, prevC2;
    while(1){
        prevC1 = c1;
        prevC2 = c2;
        c1 = fgetc(file1);
        c2 = fgetc(file2);

        if((c1 != c2)){
            theFilesAreTheSame = 0;

            if( (!((c1 == EOF) && (c2 == '\n'))) && (!((c2 == EOF) && (c1 == '\n'))) ){
                fprintf(output_buffer, "============ #<%d> ============\n", lineNum1);
                displayLineOfFile(lineNum1, file_name1);
                displayLineOfFile(lineNum2, file_name2);
            }

            while(c1 != '\n'){
                if((c1 = fgetc(file1)) == EOF){
                    break;
                }
            }
            while(c2 != '\n'){
                if((c2 = fgetc(file2)) == EOF){
                    break;
                }
            }
        }

        if(c1 == '\n') lineNum1++;
        if(c2 == '\n') lineNum2++;

        if(c1 == EOF) eof1 = 1;
        if(c2 == EOF) eof2 = 1;
        if(eof1 || eof2) break;
    }

    if(!eof1 && eof2){
        //if(prevC2 == '\n') lineNum1--;
        fprintf(output_buffer, "<<<<<<<<<<<< #%d - #%d <<<<<<<<<<<<\n", lineNum1, howManyLines(file_name1));
        while(displayLineOfFile(lineNum1, file_name1) != -1) lineNum1++;
    }
    else if(eof1 && !eof2){
        //if(prevC1 == '\n') lineNum2--;
        fprintf(output_buffer, ">>>>>>>>>>>> #%d - #%d >>>>>>>>>>>>\n", lineNum2, howManyLines(file_name2));
        while(displayLineOfFile(lineNum2, file_name2) != -1) lineNum2++;
    }
    else if(theFilesAreTheSame) display("The files are have no difference.");

    fclose(file1);
    fclose(file2);
    return 0;
}

void listdir(const char * const dir, int current_depth, int maximum_depth){ //Note that maxDepth is a previously defined macro
    //Idea from: stackoverflow.com/questions/33589672/c-print-out-directories-and-files-recursively

    if((current_depth > maximum_depth + 1) && (maximum_depth != -1)) return; //this is completed with the "continue" at the beginning of the following while loop
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if((dp = opendir(dir)) == NULL)
    {
        //fprintf(stderr,"cannot open directory: %s\n", dir);
        fprintf(output_buffer, "cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);

    int counter = 0;
    for(int i = 0; i < (current_depth - 1) * 10; i++) fprintf(output_buffer, "-");
    fprintf(output_buffer, "%s:\n", dir);

    while((entry = readdir(dp)) != NULL)
    {
        if((current_depth > maximum_depth) && (maximum_depth != -1)) continue;
        counter++;
        if(stat(entry->d_name, &statbuf) == 0) //This is always true?
        {
            if(statbuf.st_mode & S_IFDIR)
            {
                /* Found a directory, but ignore . and .. */
                if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                    continue;

                /*
                // Concatenate directory name
                int len = strlen(filepath);
                strcat(filepath, entry->d_name);
                strcat(filepath, "/");
                */

                /* Recurse at a new indent level */
                listdir(entry->d_name, current_depth+1, maximum_depth);

                /*
                //cleanup filepath
                filepath[len] = '\0';
                */

            }
            else
            {
                /*
                // Concatenate file name
                strcpy(filename, filepath);
                strcat(filename, entry->d_name);
                //puts(filename);
                */
                for(int i = 0; i < (current_depth) * 10; i++) fprintf(output_buffer, "-");
                fprintf(output_buffer, "%s\n", entry->d_name);
            }
        }
    }
    //printf("<%s>(%d)\n", dir, counter);

    chdir("..");
    closedir(dp);
}

int isArman(int nos){ //Warning: reads from lineBuff
    if(nos<2) return 0;
    for(int i = 1; i < nos; i++){
        if(!strcmp(lineBuff[i], "=D")){
            if(strcmp(lineBuff[i-1], "--str") && strcmp(lineBuff[i-1], "--str1") && strcmp(lineBuff[i-1], "--str2")){
                if(i != nos - 1){
                    return i;
                }
                else return -1;
            }
        }
    }
    return 0;
}

void phase1display(){ //prints everything into stdout, the way the project should be in phase 1.
    FILE *mainFile = fopen(OUTPUTBUFFER, "r");
    char c;
    while((c = fgetc(mainFile)) != EOF) printf("%c", c);
    fclose(mainFile);
    return;
}

int main(){
    //Every output should be saved in a file, so that we can display it easily however we want.
    output_buffer = fopen(OUTPUTBUFFER, "w");

    //Display how to use
    display("Welcome to Mirshaf's Vim.");
    display("-Notes for the find command:\n\t-Whitespace(space, tab, newline) is supported.\n\t-Wildcards can come at the beginning or the end of the string. '\\*' is supported anywhere.");
    display("-Everything uses zero-based indexing except for the -at flag and line numbers which are 1-based.");
    display("-Each command must come in exactly one line; however you can write '\\n'.");
    display("Type a command's name to see exactly how you should structure it's input.\n");

    //Create Root
    getcwd(cwd, sizeof(cwd)); //can handle dot?
    //printf("%s, %d\n", cwd, sizeof(cwd));
    strcat(cwd, "/root"); // Beware of \ vs \\ . l
    mkdir(cwd); //returns 0 if successful and -1 otherwise

    //Main loop
    while(1){
        //Display output and clean it up for the next iteration:
        fclose(output_buffer); //previous bug: close before reading from it so that it will be saved.
        phase1display();
        output_buffer = fopen(OUTPUTBUFFER, "w");

        int nos = getline(lineBuff); //Number of strings(words)
        processLine(lineBuff, nos);

        int forPipeline = isArman(nos);

        /*printf("nos: %d\n", nos);
        for(int i = 0; i < nos; i++){
            printf("<%s>", lineBuff[i]);
        }
        printf("\n");*/
        if(nos == 0) continue;
        else if(nos == -1) display("Error: Too many words in one line.");

        else if(!strcmp(lineBuff[0], "createfile")){
            if(nos != 3){
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

        else if(!strcmp(lineBuff[0], "insertstr")){
            if(nos != 8){
                display("Error: The format should be 'insertstr --file <file name> --str <str> --pos <line no>:<start position>'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[2]) == 1){
                    insertstr(lineBuff[2], lineBuff[6], lineBuff[7], lineBuff[4]);
                }
            }
        }

        else if(!strcmp(lineBuff[0], "cat")){
            if((nos != 3) && (forPipeline <= 0)){
                display("Error: The format should be 'cat --file <file name>'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[2]) == 1){
                    cat();
                }
            }
        }

        else if(!strcmp(lineBuff[0], "removestr")){
            if(nos != 9){
                display("Error: The format should be 'removestr --file <file name> --pos <line no>:<start position> -size <number of characters to remove> -f or -b'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[2]) == 1){
                    if(removestr() == 0) display("Removed successfully.");
                    //else display("Something went wrong.");
                }
            }
        }

        else if(!strcmp(lineBuff[0], "copystr")){
            if(nos != 9){
                display("Error: The format should be 'copystr --file <file name> --pos <line no>:<start position> -size <number of characters to remove> -f or -b'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[2]) == 1){
                    if(copystr() == 0) display("Copied successfully.");
                    //else display("Something went wrong.");
                }
            }
        }

        else if(!strcmp(lineBuff[0], "cutstr")){
            if(nos != 9){
                display("Error: The format should be 'cutstr --file <file name> --pos <line no>:<start position> -size <number of characters to remove> -f or -b'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[2]) == 1){
                    if(cutstr() == 0) display("Cut successfully.");
                    else display("Something went wrong.");
                }
            }
        }

        else if(!strcmp(lineBuff[0], "pastestr")){
            if(nos != 6){
                display("Error: The format should be 'pastestr --file <file name> --pos <line no>:<start position>'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[2]) == 1){
                    pastestr();
                }
            }
        }

        else if(!strcmp(lineBuff[0], "find")){
            if((nos < 5) && (forPipeline <= 0)){
                display("Error: The format should be 'find --str <str> --file <file name> [-count or -at <num> or -byword or -all or an acceptable combination of them]'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[4]) == 1){
                    findInFile(nos, lineBuff[4], lineBuff[2]);
                }
            }
        }

        else if(!strcmp(lineBuff[0], "replace")){
            if((nos != 7) && (nos != 8) && (nos != 9)){
                display("Error: The format should be 'replace --str1 <initial str> --str2 <final str> --file <file name> [-at <num> | -all]'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[6]) == 1){
                    replaceInFile(nos, lineBuff[6], lineBuff[2], lineBuff[4]);
                }
            }
        }

        else if(!strcmp(lineBuff[0], "undo")){
            if(nos != 3){
                display("Error: The format should be 'undo --file <file>'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[2]) == 1){
                    if(revertFile(lineBuff[2], directory) == 0) display("Undid successfully.");
                    else display("Error: There have been no changes to this file.");
                }
            }
        }

        else if(!strcmp(lineBuff[0], "grep")){
            if((nos < 5) && (forPipeline <= 0)){
                display("Error: The format should be 'grep [-c | -l] --str <pattern> --files [<file1> <file2> <file3> ...]'");
                continue;
            }
            else{
                int flagL = 0, flagC = 0, haveReachedFiles = 0, whereIsStr = 2, counter = 0;
                for(int i = 0; i < nos; i++){
                    if(!haveReachedFiles){
                        if(!strcmp(lineBuff[i], "-l")) flagL = 1;
                        else if(!strcmp(lineBuff[i], "-c")) flagC = 1;
                    }
                    else{
                        if(handleExistence(lineBuff[i]) == 1){
                            int temp = myGrep(flagL, flagC, lineBuff[whereIsStr], lineBuff[i]);
                            if(temp > 0){
                                counter += temp;
                            }
                        }
                    }
                    if(!strcmp(lineBuff[i], "--files")){
                        haveReachedFiles = 1;
                        if(flagL && flagC){
                            display("Error: The [-c] and [-l] flags cannot come together.");
                            break;
                        }
                        else if(flagL || flagC) whereIsStr = 3;
                    }
                }
                if(flagC && !flagL) fprintf(output_buffer, "%d\n", counter);
                if((!flagC) && (counter == 0)) display("No match found.");
            }
        }

        else if(!strcmp(lineBuff[0], "auto-indent")){
            if(nos != 2){
                display("Error: The format should be 'auto-indent <file>'");
                continue;
            }
            else{
                if(handleExistence(lineBuff[1]) == 1){
                    createBackup(lineBuff[1], directory); //For the Undo command
                    if(indentFile(lineBuff[1]) == 0) display("Done successfully.");
                    else display("An unknown error occurred.");
                }
            }
        }

        else if(!strcmp(lineBuff[0], "compare")){
            if((nos != 3) && (forPipeline <= 0)){
                display("Error: The format should be 'compare <file1> <file2>'");
                continue;
            }
            else{
                if((handleExistence(lineBuff[1]) == 1) && (handleExistence(lineBuff[2]) == 1)){
                    textComparator(lineBuff[1], lineBuff[2]);
                }
            }
        }

        else if(!strcmp(lineBuff[0], "tree")){
            if((nos != 2) && (forPipeline <= 0)){
                display("Error: The format should be 'tree <depth>'");
                continue;
            }
            else{
                int depth = atoi(lineBuff[1]);
                if(depth < -1) display("Invalid depth.");
                else{
                    listdir("root", 1, depth);
                }
            }
        }

        else if(!strcmp(lineBuff[0], "exit")){
            fclose(output_buffer);
            break;
        }

        else{
            display("invalid command");
            forPipeline = 0;
        }


        //Arman feature(pipelining commands):
        if((forPipeline != 0) && ((forPipeline = isArman(nos)) != 0)){ //The first condition ensures the first command was correct~
            if(forPipeline == -1){
                display("Error: '=D' cannot come at the end of the line.");
                continue;
            }

            //transfer the output from getting displayed to being passed to a command:
            fclose(output_buffer);
            output_buffer = fopen(OUTPUTBUFFER, "r");
            int limit = maxCharInAStr, currentChar = 0;
            char fileContents[limit], c;
            while((c = fgetc(output_buffer)) != EOF){
                if(currentChar >= (limit-1)) break;
                fileContents[currentChar] = c;
                currentChar++;
            }
            fileContents[currentChar] = '\0';
            fclose(output_buffer);
            output_buffer = fopen(OUTPUTBUFFER, "w");


            if(!strcmp(lineBuff[forPipeline+1], "insertstr")){
                if(nos != (forPipeline + 7)){
                    display("Error: The format should be '<some command> =D insertstr --file <file name> --pos <line no>:<start position>'");
                    continue;
                }
                else{
                    if(handleExistence(lineBuff[forPipeline+3]) == 1){
                        insertstr(lineBuff[forPipeline+3], lineBuff[forPipeline+5], lineBuff[forPipeline+6], fileContents);
                    }
                }
            }

            else if(!strcmp(lineBuff[forPipeline+1], "find")){
                if(nos < forPipeline + 4){
                    display("Error: The format should be '<some command> =D find --file <file name> [-count or -at <num> or -byword or -all or an acceptable combination of them]'");
                    continue;
                }
                else{
                    if(handleExistence(lineBuff[forPipeline+3]) == 1){
                        findInFile(nos, lineBuff[forPipeline+3], fileContents);
                    }
                }
            }

            else if(!strcmp(lineBuff[forPipeline+1], "replace")){
                if((nos != forPipeline + 6) && (nos != forPipeline + 7) && (nos != forPipeline + 8)){
                    display("Error: The format should be '<some command> =D replace --str2 <final str> --file <file name> [-at <num> | -all]' (you could pass --str1 instead of --str2)");
                    continue;
                }
                else{
                    if(handleExistence(lineBuff[forPipeline+5]) == 1){
                        int isStr1missing = 1;
                        for(int i = 0; i < nos; i++){
                            if(!strcmp(lineBuff[i], "--str1")){
                                isStr1missing = 0;
                                break;
                            }
                        }
                        if(isStr1missing) replaceInFile(nos, lineBuff[forPipeline+5], fileContents, lineBuff[forPipeline+3]);
                        else replaceInFile(nos, lineBuff[forPipeline+5], lineBuff[forPipeline+3], fileContents);
                    }
                }
            }

            else if(!strcmp(lineBuff[forPipeline+1], "grep")){
                if((nos < forPipeline + 4)){
                    display("Error: The format should be '<some command> =D grep [-c | -l] --files [<file1> <file2> <file3> ...]'");
                    continue;
                }
                else{
                    int flagL = 0, flagC = 0, haveReachedFiles = 0, counter = 0;
                    for(int i = 0; i < nos; i++){
                        if(!haveReachedFiles){
                            if(!strcmp(lineBuff[i], "-l")) flagL = 1;
                            else if(!strcmp(lineBuff[i], "-c")) flagC = 1;
                        }
                        else{
                            if(handleExistence(lineBuff[i]) == 1){
                                int temp = myGrep(flagL, flagC, fileContents, lineBuff[i]);
                                if(temp > 0){
                                    counter += temp;
                                }
                            }
                        }
                        if(!strcmp(lineBuff[i], "--files")){
                            haveReachedFiles = 1;
                            if(flagL && flagC){
                                display("Error: The [-c] and [-l] flags cannot come together.");
                                break;
                            }
                        }
                    }
                    if(flagC && !flagL) fprintf(output_buffer, "%d\n", counter);
                    if((!flagC) && (counter == 0)) display("No match found.");
                }
            }

            else {
                display("Error: The command after arman(=D) must take a string as an input.");
            }
        }
    }

    return 0;
}






