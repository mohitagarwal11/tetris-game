#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define HEIGHT (21)
#define WIDTH (11)

int tetroType[7] ={2,4,6,0,1,3,5};
int tetroIndex=7;

typedef struct{
    int x,y;
}position;

typedef struct{
    position cells[4]; //each has 4sqaures with xy
    int type;          //6 types of tetromino
}tetromino;

void gotoxy(int x, int y){
    printf("\033[%d;%dH",x,y);
    fflush(stdout);
}

void initTerminal() {
    struct termios term;
    tcgetattr(0,&term);
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(0,TCSANOW,&term);
}

void resetTerminal() {
    struct termios term;
    tcgetattr(0,&term);
    term.c_lflag |= ICANON|ECHO;
    tcsetattr(0,TCSANOW,&term);
}

void setNonBlockingInput() {
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK); 
}

void hideCursor() {
    printf("\e[?25l");
    fflush(stdout);
}

void showCursor() {
    printf("\e[?25h");
    fflush(stdout);
}
//sm famous guys shuffe theorem to shuffle tetrominos in arr
void shuffleTetroType(){
    for(int i=0;i<7;i++){
        int j = rand() % 7;
        int temp = tetroType[i];
        tetroType[i] = tetroType[j];
        tetroType[j] = temp;
    }
    tetroIndex=0;
}

int getNextTetroType(){
    if(tetroIndex>=7) shuffleTetroType();
    return tetroType[tetroIndex++];
}

tetromino generateRandomTetromino(){
    tetromino t;
    t.type = getNextTetroType();
    switch(t.type){
        case 0: //S
            t.cells[0] = (position){0,1};
            t.cells[1] = (position){0,2};
            t.cells[2] = (position){1,0};
            t.cells[3] = (position){1,1};
            break;
        case 1: //T
            t.cells[0] = (position){0,0};
            t.cells[1] = (position){0,1};
            t.cells[2] = (position){0,2};
            t.cells[3] = (position){1,1};
            break;
        case 2: //opp S
            t.cells[0] = (position){0,0};
            t.cells[1] = (position){0,1};
            t.cells[2] = (position){1,1};
            t.cells[3] = (position){1,2};
            break;
        case 3: //L
            t.cells[0] = (position){0,0};
            t.cells[1] = (position){1,0};
            t.cells[2] = (position){2,0};
            t.cells[3] = (position){2,1};
            break;
        case 4: //l
            t.cells[0] = (position){0,0};
            t.cells[1] = (position){1,0};
            t.cells[2] = (position){2,0};
            t.cells[3] = (position){3,0};
            break;
        case 5: //O
            t.cells[0] = (position){0,0};
            t.cells[1] = (position){0,1};
            t.cells[2] = (position){1,0};
            t.cells[3] = (position){1,1};
            break;
        case 6: //opp L
            t.cells[0] = (position){0,1};
            t.cells[1] = (position){1,1};
            t.cells[2] = (position){2,0};
            t.cells[3] = (position){2,1};
            break;
    }
    return t;
}

int canMove(tetromino t, position block, int x, int y, int grid[][WIDTH]){
    for(int k=0;k<4;k++){
        int cellX = block.x + t.cells[k].x + x;//x is move direc value top to bottom
        int cellY = block.y + t.cells[k].y + y;//y is same for left to right
        
        if(cellX >= HEIGHT || cellY >= WIDTH || cellY < 1)
            return 0;//dont allow
        if(grid[cellX][cellY]) return 0;// dont allow bcuz collision
    }
    return 1;
}

tetromino rotateTetromino(tetromino t){
    tetromino rotated=t;
    for(int k=0;k<4;k++){
        int tempX = t.cells[k].x;
        int tempY = t.cells[k].y;
        rotated.cells[k].x = t.cells[k].y;
        rotated.cells[k].y = 3-tempX;
    }
    return rotated;
}

int clearLines(int grid[][WIDTH], int linesCleared){
    for(int i=HEIGHT-1; i>=1; i--){
        int isFull=1;
        for(int j=1; j<=WIDTH-1; j++){
            if(!grid[i][j]){
                isFull=0;
                break;
            }
        }
        if(isFull){
            for(int row=i;row>=1;row--){
                for(int col=1;col<=WIDTH-1;col++){
                    grid[row][col] = grid[row-1][col];//shift down
                }
            }
            i++;
            linesCleared++;
        }
    }
    return linesCleared;
}

void gameOver(int score){
    printf("\n\t  GAME OVER!\n");
    printf("\t  Score: %d\n",score);
    showCursor();
    resetTerminal();
    exit(0);
}

void saveGrid(tetromino tetro, position block, int grid[][WIDTH]){
    //to save the tetros falling and stuck on the grid
    for (int k = 0; k < 4; k++) {
        int cellX = block.x + tetro.cells[k].x;
        int cellY = block.y + tetro.cells[k].y;

        if (cellX >= 0 && cellX < HEIGHT && cellY >= 0 && cellY < WIDTH){
            grid[cellX][cellY] = 1;
        }   
    }
}

void drawWindow(position block, position nextBlock, tetromino t, tetromino nextT, int grid[][WIDTH], int score, int linesCleared){

    printf("\033[2J\033[H"); //linux clear screen
    
    position ghost = block;
    while(canMove(t, ghost, 1, 0, grid)) ghost.x++;
    
    printf("\nWelcome to my Tetris Game!\n");
    printf("Use 'A','S','D' for movement and 'Q' to exit.\n");
    printf("Use 'L' for rotation.\n");
    printf("Use 'W' to drop instantly!\n");
    printf("\t\t\t~MAW11");

    printf("\nNext:");
    for(int k=0;k<4;k++){
        int cellX = nextBlock.x + nextT.cells[k].x;
        int cellY = nextBlock.y + nextT.cells[k].y;
        gotoxy(cellX,cellY);
        printf("OO");
    }
    printf("\n\n");

    for(int i=0;i<=HEIGHT;i++){
        for(int j=0;j<=WIDTH;j++){
            int printed=0;

            if(i==HEIGHT || j==WIDTH || i==0 || j==0){
                printf("XX");
                printed=1;
            }else{
                for(int k=0;k<4;k++){
                    int cellX = block.x + t.cells[k].x;
                    int cellY = block.y + t.cells[k].y;
                    if(i==cellX && j==cellY){
                        printf("[]");
                        printed=1;
                        break;
                    }
                }
                if(!printed){
                    for(int k=0;k<4;k++){
                        int cellX = ghost.x + t.cells[k].x;
                        int cellY = ghost.y + t.cells[k].y;
                        if(i==cellX && j==cellY){
                            printf("::");
                            printed=1;
                            break;
                        }
                    }   
                }
            }

            if(!printed && grid[i][j]){
                printf("[]");
                printed=1;
            }
            if(!printed) printf("  ");
        }
        printf("\n");
    }
    printf("Score: %d\n", score);
    printf("Lines Cleared: %d\n",linesCleared);
}


int main(){
    srand(time(NULL));
    
    initTerminal(); 
    setNonBlockingInput(); 
    hideCursor();

    int grid[HEIGHT][WIDTH] = {0};
    int score=0;
    int linesCleared=0;
    int tetroCnt=0;

    position block = {1, WIDTH/2};
    position nextBlock = {7,7};

    char prevInput=0;
    char key=0;

    tetromino tetro = generateRandomTetromino();
    tetromino rotated;
    tetromino nextTetro = generateRandomTetromino();

    //up1: linux input is much btr than win now after using previnput method and changing fps checks
    while(1){
        key=0;
        read(0, &key, 1);
        
        if (key!=0 && key!=prevInput) {
            prevInput = key;
            key = tolower(key);
            switch (key) {
                case 'a':
                    if (canMove(tetro,block,0,-1,grid)) block.y--;
                    break;
                case 'd':
                    if (canMove(tetro,block,0,1,grid)) block.y++; 
                    break;
                case 's':
                    if (canMove(tetro,block,1,0,grid)) block.x++;
                    break;
                case 'q':
                    gameOver(score);
                    break;
                case 'l':
                    rotated = rotateTetromino(tetro);
                    if(canMove(rotated, block, 0, 0, grid)) {
                        tetro = rotated;
                    }
                    break;
                case 'w':
                    while (canMove(tetro, block, 1, 0, grid)) {
                        block.x++;//free fall or direct drop
                    }
                    break;
            }
        }

        //clear key on no press to remove the single side onetime movement
        if (key == 0) prevInput = 0;

        static int frame = 0;
        if (++frame % 30 == 0) { //every 30 frame we check/change block height
            if (block.x < HEIGHT - 1) {
                block.x++;
            } else { //if it is equal to height-1 then a new block is instantiated at top
                tetro = nextTetro;
                nextTetro =  generateRandomTetromino();
                tetroCnt +=10;
                block.x = 1;
                block.y = WIDTH / 2;
            }
            frame = 0;
        }

        if (!canMove(tetro, block, 1, 0, grid)){
            saveGrid(tetro, block, grid);
            
            tetroCnt += 10;
            tetro = nextTetro;
            nextTetro =  generateRandomTetromino();
            block.x = 1;
            block.y = WIDTH / 2;

            if (!canMove(tetro, block, 0, 0, grid)) {
                gameOver(score);
            }   
        }
        
        linesCleared = clearLines(grid, linesCleared);
        score = tetroCnt + (100*linesCleared);
        drawWindow(block, nextBlock, tetro, nextTetro, grid, score, linesCleared);
        usleep(25000);
    }
    return 0;
}