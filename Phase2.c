//Pay respect to: www.cyberciti.biz/faq/linux-install-ncurses-library-headers-on-debian-ubuntu-centos-fedora/
//Pay respect to: https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <sys/ioctl.h> //to know the max width and height

#define PHASE2BUFFER "phase2buffer.txt"
#define maxCharInAStr 450


#define BASEROW 1
#define BASECOL 5 //border:1 lineNumber:4
#define LINENUMBER 1
int row = BASEROW, col = BASECOL, max_row, max_col, current_line = 2;

enum vim_mode {VIM_INSERT, VIM_NORMAL, VIM_VISUAL}; //length of str should be 9
enum vim_mode current_mode = VIM_NORMAL;

int is_saved = 1;

int isWritingCommand = 0;
FILE *commandBar;
#define CommandbarFile "commandbar.txt"

WINDOW * mainwin;

int cursor_row = BASEROW, cursor_col = BASECOL;

//visual mode:
int icr, icc; //initial_cursor_row and col

void phase2display(const char file_name[maxCharInAStr], int row, int col, int current_line){
    //Erase:
    erase();
    box(mainwin, 0, 0);


    FILE *mainFile = fopen(file_name, "r");
    char c;

    mvaddstr(BASEROW, LINENUMBER, "1");
    //move(row, col);
    while((c = fgetc(mainFile)) != EOF){
        //addch(temp);
        if(current_mode == VIM_VISUAL){
            /*if(cursor_row > icr){
                if((row > icr) || ((row == icr) && (col >= icc))){
                    attron(A_BLINK);
                    mvaddch(row, col, c);
                    attroff(A_BLINK);
                }
                else mvaddch(row, col, c);
            }
            else if(cursor_row < icr){
                if((row < icr) || ((row == icr) && (col <= icc))){
                    attron(A_BLINK);
                    mvaddch(row, col, c);
                    attroff(A_BLINK);
                }
                else mvaddch(row, col, c);
            }*/
            if(cursor_row == icr){
                if((((icc >= col) && (col >= cursor_col)) || ((icc <= col) && (col <= cursor_col))) && (row == icr)){
                    start_color();
                    init_pair(1, COLOR_WHITE, COLOR_BLACK);
                    init_pair(2, COLOR_BLACK, COLOR_WHITE);
                    attron(COLOR_PAIR(2));
                    mvaddch(row, col, c);
                    attroff(COLOR_PAIR(1));
                }
                else mvaddch(row, col, c);
            }
            else{
                mvaddch(row, col, c);
            }
        }
        else mvaddch(row, col, c);
        if(c == '\n'){
            if(row == cursor_row){
                if(cursor_col >= col) cursor_col = col - 1;
            }

            row++;
            col = BASECOL;

            //Line number:
            char tempc[BASECOL]; //BASECOL - border + '\0'
            sprintf(tempc, "%d", current_line);
            mvaddstr(row, LINENUMBER, tempc);
            current_line++;
        }
        else if(c == '\t'){
            col += 4;
        }
        else{
            col++;
        }

        if(col > (max_col - BASECOL - 1)){
            row++;
            col = BASECOL;

            char tempc[BASECOL];
            sprintf(tempc, "%d", current_line);
            mvaddstr(row, LINENUMBER, tempc);
            current_line++;

        }
        if(row >= max_row - 3){
            //To be implemented
        }
    }
    while(row < max_row - 4){
        if(row == cursor_row){
            cursor_col = BASECOL;
        }

        //Line number:
        mvaddstr(row, LINENUMBER, "~");
        current_line++;
        row++;
    }

    //Mode:
    char tempstr[10] = "AH";
    if(current_mode == VIM_NORMAL){
        strcpy(tempstr, "NORMAL");
    }
    else if(current_mode == VIM_INSERT){
        strcpy(tempstr, "INSERT");
    }
    else if(current_mode == VIM_VISUAL){
        strcpy(tempstr, "VISUAL");
    }
    mvaddstr(max_row-3, LINENUMBER + 1, tempstr);

    //File name:
    mvaddstr(max_row-3, LINENUMBER + 11, file_name);
    if(!is_saved) mvaddch(max_row - 3, LINENUMBER + 11 + strlen(file_name) + 1, '+');

    //CommandBar:
    int tempCol = LINENUMBER + 1;
    commandBar = fopen(CommandbarFile, "r");
    while((c = fgetc(commandBar)) != EOF){
        mvaddch(max_row-2, tempCol, c);
        tempCol++;
    }
    fclose(commandBar);

    move(cursor_row, cursor_col);
    fclose(mainFile);
    return;
}

int main(void) {
    int ch;
    /*  Initialize ncurses  */
    if ( (mainwin = initscr()) == NULL ) {
	fprintf(stderr, "Error initialising ncurses.\n");
	exit(EXIT_FAILURE);
    }
    /*  Switch of echoing and enable keypad (for arrow keys)  */
    noecho();
    keypad(mainwin, TRUE);

    //Setting up:
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    //printf("\t\t%d %d\t\t", w.ws_row, w.ws_col); //Do not use \n in printf
    max_row = w.ws_row;
    max_col = w.ws_col;
    commandBar = fopen(CommandbarFile, "w");
    fprintf(commandBar, "CommandBar");
    fclose(commandBar);



    phase2display(PHASE2BUFFER, row, col, current_line);
    refresh();


    /*  Loop until user hits 'q' to quit  */

    while ( ch = getch() ) {
        max_row = w.ws_row;
        max_col = w.ws_col;

        //CommandBar:
        if(isWritingCommand){
            if(ch == '\n'){
                isWritingCommand = 0;

                //Check the command:
                char strCommand[150], tempc;
                int strindex = 0;
                commandBar = fopen(CommandbarFile, "r");
                while((tempc = fgetc(commandBar)) != EOF){
                    strCommand[strindex] = tempc;
                    strindex++;
                }
                strCommand[strindex] = '\0';
                fclose(commandBar);
                if(!strcmp(strCommand, "i")){ //INSERT MODE
                    current_mode = VIM_INSERT;
                }
                else if(!strcmp(strCommand, "n")){ //NORMAL MODE
                    current_mode = VIM_NORMAL;
                }
                else if(!strcmp(strCommand, "v")){ //VISUAL MODE
                    current_mode = VIM_VISUAL;
                    icr = cursor_row;
                    icc = cursor_col;
                }
            }
            else{
                commandBar = fopen(CommandbarFile, "a");
                fputc(ch, commandBar);
                fclose(commandBar);
            }
        }
        else{
            if((ch == 'q')){
                break;
            }

            if(current_mode == VIM_NORMAL){
                if(ch == 'k'){ //up
                    if(cursor_row > BASEROW){
                        cursor_row--;
                    }
                }
                else if(ch == 'j'){ //down
                    if(cursor_row < max_row - 5){
                        cursor_row++;
                    }
                }
                else if(ch == 'h'){ //left
                    if(cursor_col > BASECOL){
                        cursor_col--;
                    }
                }
                else if(ch == 'l'){ //right{
                    if(cursor_col < max_col - BASECOL - 1){
                        cursor_col++;
                    }
                }

                else if((ch == ':') || (ch == '/')){ //Initiate writing in commandbar
                    isWritingCommand = 1;
                    commandBar = fopen(CommandbarFile, "w");
                    //fprintf(commandBar, "Enter a command...");
                    fclose(commandBar);
                }
            }
            else{ //PRESS N TO Go BACK TO NORMAL MODE
                if(ch == 'n'){
                    current_mode = VIM_NORMAL;
                }

                if(current_mode == VIM_VISUAL){
                    if(ch == 'k'){ //up
                        if(cursor_row > BASEROW){
                            cursor_row--;
                        }
                    }
                    else if(ch == 'j'){ //down
                        if(cursor_row < max_row - 5){
                            cursor_row++;
                        }
                    }
                    else if(ch == 'h'){ //left
                        if(cursor_col > BASECOL){
                            cursor_col--;
                        }
                    }
                    else if(ch == 'l'){ //right{
                        if(cursor_col < max_col - BASECOL - 1){
                            cursor_col++;
                        }
                    }
                }
            }
        }

        //Refresh:
        phase2display(PHASE2BUFFER, row, col, current_line);
        refresh();
    }


    /*  Clean up after ourselves  */

    delwin(mainwin);
    endwin();
    refresh();

    return EXIT_SUCCESS;
}

