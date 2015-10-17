#include <stdlib.h>
#include <string.h>
#include "myLib.h"
#include "text.h"
//#include "kit.h"


#include "Pokemon_RBY_PalletTown.h"
#include "collision.h"
#include "floatingText.h"
#include "walk.h"
#include "sprites.h"
#include "opening2.h"
#include "gameMusic.h"
#include "victory.h"
#include "collide.h"
#include "defeat.h"

#define BLACKINDEX 0
#define REDINDEX 1
#define BLUEINDEX 2
#define GREENINDEX 3
#define WHITEINDEX 4

#define ROWMASK (0xFF)
#define COLMASK (0x1FF)

#define SPRITEOFFSET16(r,c) (r)*32+(c)
ObjAttr shadowOAM[128];

int i;
int r;

int changeDir = 0;
int previousDir = 3;
int direction = 3;
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3


typedef struct{
    unsigned char* data;
    int length;
    int frequency;
    int isPlaying;
    int loops;
    int duration;
}SOUND;

SOUND soundA;
SOUND soundB;

int vbCountA;
int vbCountB;
int vbCountAnimate;
int animateCount;
int modeChange = 0;

void initialize();
void update();
void draw();

#define STARTSCREEN 0
#define GAMESCREEN 1
#define LOSESCREEN 2
#define WINSCREEN 3
#define PAUSESCREEN 4
#define INSTRUCTIONSSCREEN 5

int state;

void setupSounds();
void playSoundA( const unsigned char* sound, int length, int frequency, int isLooping);
void playSoundB( const unsigned char* sound, int length, int frequency, int isLooping);
void muteSound();
void unmuteSound();
void stopSounds();
void setupInterrupts();
void interruptHandler();

void start();
void game();
void pause();
void win();
void lose();
void instructions();

//Movement
void up();
void down();
void left();
void right();

int randomSeed = 0;
int cheat = 0;

unsigned int buttons;
unsigned int oldButtons;

unsigned short scanLineCounter;
char fpsbuffer[30];

#define BLACKINDEX 0
#define REDINDEX 1
#define BLUEINDEX 2
#define GREENINDEX 3
#define WHITEINDEX 4

int hOffZero;
int vOffZero;
int hOffOne;

typedef struct {
    int row;
    int col;
    int bigRow;
    int bigCol;
    int start;
    int shape;
    int size;
    int height;
    int width;
    int rdel;
    int cdel;
} Sprite;
#define NUMSPRITES 7
#define NUMCOMPUTERS 6

Sprite redSprite;
Sprite computers[NUMCOMPUTERS];
Sprite arrow;
//Sprite helix;


//= {32,32,SPRITEOFFSET16(0,0),ATTR0_SQUARE,ATTR1_SIZE16,16,16,1,1}; 
//{32, 32, SPRITEOFFSET16(0,0), ATTR0_SQUARE, ATTR1_SIZE16, 16, 16, 0, 0};
//Sprite computer = {64,64,SPRITEOFFSET16(0,1),ATTR0_TALL,ATTR1_SIZE16,16,32,1,1};

int main()
{
        REG_DISPCTL = BG2_ENABLE | MODE4;
        PALETTE[BLACKINDEX] = BLACK;
	PALETTE[REDINDEX] = RED;
	PALETTE[BLUEINDEX] = BLUE;
	PALETTE[GREENINDEX] = GREEN;
	PALETTE[WHITEINDEX] = WHITE;
        initialize();
        
	
        //playSoundA(TitleSong,TITLESONGLEN,TITLESONGFREQ, 1);
        while(1)
	{
            oldButtons = buttons;
            buttons = BUTTONS;
		
            switch(state)
            {
                    case STARTSCREEN:
                            start();
                            break;
                    case GAMESCREEN:
                            game();
                            break;
                    case PAUSESCREEN:
                            pause();
                            break;
                    case WINSCREEN:
                            win();
                            break;
                    case LOSESCREEN:
                            lose();
                            break;
                    case INSTRUCTIONSSCREEN:
                            instructions();
                            break;
            }
                
                
            REG_BG1HOFS = hOffZero;
            REG_BG1VOFS = vOffZero;
            REG_BG0HOFS = hOffOne;
            waitForVblank();
	}

	return 0;
}

void setupSounds()
{
        REG_SOUNDCNT_X = SND_ENABLED;

	REG_SOUNDCNT_H = SND_OUTPUT_RATIO_100 | 
                        DSA_OUTPUT_RATIO_100 | 
                        DSA_OUTPUT_TO_BOTH | 
                        DSA_TIMER0 | 
                        DSA_FIFO_RESET |
                        DSB_OUTPUT_RATIO_100 | 
                        DSB_OUTPUT_TO_BOTH | 
                        DSB_TIMER1 | 
                        DSB_FIFO_RESET;

	REG_SOUNDCNT_L = 0;
}

void playSoundA( const unsigned char* sound, int length, int frequency, int isLooping) {

	
        dma[1].cnt = 0;
        vbCountA = 0;
	
        int interval = 16777216/frequency;
	
        DMANow(1, sound, REG_FIFO_A, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);
	
        REG_TM0CNT = 0;
	
        REG_TM0D = -interval;
        REG_TM0CNT = TIMER_ON;
	
        soundA.data = sound;
        soundA.length = length;
        soundA.frequency = frequency;
        soundA.isPlaying = 1;
        soundA.loops = isLooping;
        soundA.duration = ((60*length)/frequency) - ((length/frequency)*3)-1;
}


void playSoundB( const unsigned char* sound, int length, int frequency, int isLooping) {

        dma[2].cnt = 0;
        vbCountB = 0;

        int interval = 16777216/frequency;

        DMANow(2, sound, REG_FIFO_B, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);

        REG_TM1CNT = 0;
	
        REG_TM1D = -interval;
        REG_TM1CNT = TIMER_ON;
	
	soundB.data = sound;
        soundB.length = length;
        soundB.frequency = frequency;
        soundB.isPlaying = 1;
        soundB.loops = isLooping;
        soundB.duration = ((60*length)/frequency) - ((length/frequency)*3)-1;
}
void muteSound()
{
    REG_SOUNDCNT_X = 0;
}

void unmuteSound() 
{
    REG_SOUNDCNT_X = SND_ENABLED;
}

void stopSounds()
{
    dma[1].cnt = 0;
    dma[2].cnt = 0;
}
void setupInterrupts()
{
	REG_IME = 0;
	REG_INTERRUPT = (unsigned int)interruptHandler;
	REG_IE |= INT_VBLANK;
	REG_DISPSTAT |= INT_VBLANK_ENABLE;
	REG_IME = 1;
}

void interruptHandler()
{
	REG_IME = 0;
	if(REG_IF & INT_VBLANK)
	{
                
            vbCountA++;
            vbCountB++;
            vbCountAnimate++;
            changeDir++;
            hOffOne++;
            if(changeDir >= 50)
            {
                previousDir = direction;
                while(direction == previousDir)
                {
                    direction = rand()%4;
                }
                switch(direction)
                {
                        case UP:
                                arrow.start = SPRITEOFFSET16(0,10);
                                break;
                        case DOWN:
                                arrow.start = SPRITEOFFSET16(0,14);
                                break;
                        case LEFT:
                                arrow.start = SPRITEOFFSET16(0,12);
                                break;
                        case RIGHT:
                                arrow.start = SPRITEOFFSET16(0,8);
                                break;
                }
                changeDir = 0;
            }
            if(vbCountAnimate >= 100)
            {
                if(!animateCount)
                {
                    for(i=0; i<NUMCOMPUTERS; i++)
                    {
                        computers[i].start = SPRITEOFFSET16(0,4);
                        animateCount = 1;
                    }
                }
                else
                {
                    for(i=0; i<NUMCOMPUTERS; i++)
                    {
                        computers[i].start = SPRITEOFFSET16(0,2);
                        animateCount = 0;
                    }
                }
                vbCountAnimate = 0;
                
            }
            if(vbCountA >= soundA.duration) 
            {
                
                
                if (soundA.loops)
                {
                    playSoundA(soundA.data, soundA.length, soundA.frequency, soundA.loops);
                    vbCountA = 0;
                    
                }
                else 
                {
                    REG_TM0CNT = 0;
                    soundA.data = 0;
                    soundA.length = 0;
                    soundA.frequency = 0;
                    soundA.isPlaying = 0;
                    soundA.loops = 0;
                    soundA.duration = 0;
                    
                }
            }
            if(vbCountB >= soundB.duration) 
            {
                REG_TM1CNT = 0;
                if (soundB.loops) 
                {
                    playSoundB(soundB.data, soundB.length, soundB.frequency, soundB.loops);
                    
                }
                else
                {
                    soundB.data = 0;
                    soundB.length = 0;
                    soundB.frequency = 0;
                    soundB.isPlaying = 0;
                    soundB.loops = 0;
                    soundB.duration = 0;
                    
                }
                
                
            }
		REG_IF = INT_VBLANK; 
	}

	REG_IME = 1;
}

void setUpMode0()
{
    REG_DISPCTL = MODE0 | BG0_ENABLE | BG1_ENABLE | OBJ_ENABLE;
    REG_BG1CNT = BG_SIZE0 | CBB(0) | SBB(24);
    REG_BG0CNT = BG_SIZE0 | CBB(1) | SBB(30);
    loadPalette(Pokemon_RBY_PalletTownPal);
    DMANow(3, &Pokemon_RBY_PalletTownTiles, &CHARBLOCKBASE[0], Pokemon_RBY_PalletTownTilesLen | DMA_16 | DMA_ON);
    DMANow(3, &Pokemon_RBY_PalletTownMap, &SCREENBLOCKBASE[24], Pokemon_RBY_PalletTownMapLen | DMA_16 | DMA_ON);
    DMANow(3, &floatingTextTiles, &CHARBLOCKBASE[1], floatingTextTilesLen | DMA_16 | DMA_ON);
    DMANow(3, &floatingTextMap, &SCREENBLOCKBASE[30], floatingTextMapLen | DMA_16 | DMA_ON);
    for(i = 0; i<256; i++) {
    	SPRITEPAL[i] = spritesPal[i];
    }
    
    //load sprite images
    for(i=0; i<32768/2; i++) {
    	CHARBLOCKBASE[4].tileimg[i] = spritesTiles[i];
    }
    
   
}

void start()
{
       
        fillScreen4(BLACKINDEX);
        drawString4(50, 30, "Twitch Plays Pokemon: THE GAME!", WHITEINDEX);
        drawString4(70, 30, "Start: Begin", WHITEINDEX);
        drawString4(90, 30, "Select: Instructions", WHITEINDEX);
        
	if(BUTTON_PRESSED(BUTTON_START))
	{
            playSoundA(gameMusic, GAMEMUSICLEN, GAMEMUSICFREQ, 1);
            state = GAMESCREEN;
            setUpMode0();
	}
        if(BUTTON_PRESSED(BUTTON_SELECT))
	{
            fillScreen4(BLACKINDEX);
            state = INSTRUCTIONSSCREEN;
	}
        flipPage();
}

void instructions()
{
    fillScreen4(BLACKINDEX);
    drawString4(40,50, "INSTRUCTIONS:", GREENINDEX);
    drawString4(50,50, "-A: TO MOVE IN THE", GREENINDEX);
    drawString4(60,50, " DIRECTION OF THE ARROW", GREENINDEX);
    drawString4(70,50, "-L: ENABLE CHEAT", GREENINDEX);
    drawString4(80,50, "(CHEATING LETS YOU USE ", GREENINDEX);
    drawString4(90,50, "  THE ARROW KEYS TO MOVE)", GREENINDEX);
    drawString4(100,50, "-AVOID BILL's EVIL PC'S", GREENINDEX);
    drawString4(110,50, "-FIND THE HELIX FOSSIL", GREENINDEX);
    drawString4(120,50, "-Press START to return", GREENINDEX);
    if(BUTTON_PRESSED(BUTTON_START))
    {
        state = STARTSCREEN;
    }
    flipPage();
}

void game()
{
        redSprite.bigCol = redSprite.col + hOffZero;
        redSprite.bigRow = redSprite.row + vOffZero;
        for (i = 0; i < NUMCOMPUTERS; i++)
        {
            computers[i].row = computers[i].bigRow - vOffZero;
            computers[i].col = computers[i].bigCol - hOffZero;
        }
        //Movement
        if(cheat)
        {
            if(BUTTON_PRESSED(BUTTON_UP))
            {
                up();
            }
            if(BUTTON_PRESSED(BUTTON_DOWN))
            {
                down();
            }
            if(BUTTON_PRESSED(BUTTON_LEFT))
            {
                left();
            }
            if(BUTTON_PRESSED(BUTTON_RIGHT))
            {
                right();
            }
        }
        
	if(BUTTON_PRESSED(BUTTON_START))
	{
            muteSound();
            
            state = PAUSESCREEN;
            REG_DISPCTL = BG2_ENABLE | MODE4;
            PALETTE[BLACKINDEX] = BLACK;
            PALETTE[REDINDEX] = RED;
            PALETTE[BLUEINDEX] = BLUE;
            PALETTE[GREENINDEX] = GREEN;
            PALETTE[WHITEINDEX] = WHITE;
	}
        
        for(i = 0; i < NUMCOMPUTERS; i++)
        {
            if(redSprite.bigCol == computers[i].bigCol &&(((redSprite.bigRow+4) == computers[i].bigRow) || ((redSprite.bigRow+4) == (computers[i].bigRow+16))))
                {
                    state = LOSESCREEN;
                    REG_DISPCTL = BG2_ENABLE | MODE4;
                    PALETTE[BLACKINDEX] = BLACK;
                    PALETTE[REDINDEX] = RED;
                    PALETTE[BLUEINDEX] = BLUE;
                    PALETTE[GREENINDEX] = GREEN;
                    PALETTE[WHITEINDEX] = WHITE;
                    playSoundA(defeat, DEFEATLEN, DEFEATFREQ, 1);
                }
        }
        
        if(redSprite.bigCol >= (14*16) && redSprite.bigCol <= (14*16) && (redSprite.bigRow+4) >= 14*16 
                && (redSprite.bigRow+4) <= 14*16)
        {
            state = WINSCREEN;
            REG_DISPCTL = BG2_ENABLE | MODE4;
            PALETTE[BLACKINDEX] = BLACK;
            PALETTE[REDINDEX] = RED;
            PALETTE[BLUEINDEX] = BLUE;
            PALETTE[GREENINDEX] = GREEN;
            PALETTE[WHITEINDEX] = WHITE;
            playSoundA(victory, VICTORYLEN, VICTORYFREQ, 1);
            
        }
        if(BUTTON_PRESSED(BUTTON_L))
	{
            if(cheat)
            {
                cheat = 0;
            }
            else
            {
                cheat = 1;
            }
        }
        if(BUTTON_PRESSED(BUTTON_A))
	{
            switch(direction)
            {
                    case UP:
                            up();
                            break;
                    case DOWN:
                            down();
                            break;
                    case LEFT:
                            left();
                            break;
                    case RIGHT:
                            right();
                            break;
            }
	}
        
        shadowOAM[0].attr0 = (ROWMASK&redSprite.row) | ATTR0_4BPP | redSprite.shape;
        shadowOAM[0].attr1 = (COLMASK&redSprite.col) | redSprite.size;
        shadowOAM[0].attr2 = redSprite.start | ATTR2_PALETTE_BANK(0);
        shadowOAM[1].attr0 = (ROWMASK&arrow.row) | ATTR0_4BPP | arrow.shape;
        shadowOAM[1].attr1 = (COLMASK&arrow.col) | arrow.size;
        shadowOAM[1].attr2 = arrow.start | ATTR2_PALETTE_BANK(0);
        for(i=0; i<NUMCOMPUTERS; i++)
        {
            shadowOAM[2+i].attr0 = (ROWMASK&computers[i].row) | ATTR0_4BPP | computers[i].shape;
            shadowOAM[2+i].attr1 = (COLMASK&computers[i].col) | computers[i].size;
            shadowOAM[2+i].attr2 = computers[i].start | ATTR2_PALETTE_BANK(0);
        }
        
        for(i=2+NUMCOMPUTERS; i<128; i++) {
                shadowOAM[i].attr0 = ATTR0_HIDE;
        }
        DMANow(3, shadowOAM, SPRITEMEM, 512 | DMA_ON);

}

void pause()
{
        
        fillScreen4(BLACKINDEX);
        drawString4(50,50, "START: Return", GREENINDEX);
        drawString4(70,50, "SELECT: Quit", GREENINDEX);
        if(BUTTON_PRESSED(BUTTON_START))
        {
            unmuteSound();
            setUpMode0();
            state = GAMESCREEN;
        }
        if(BUTTON_PRESSED(BUTTON_SELECT))
	{
            stopSounds();
            fillScreen4(BLACKINDEX);
            initialize();
            state = STARTSCREEN;
	}
        flipPage();

}

void win()
{
        
        fillScreen4(BLACKINDEX);
        drawString4(50,50, "OMANYTE HAS BEEN REVIVED!", GREENINDEX);
        drawString4(70, 50, "ALL HAIL THE HELIX FOSSIL", GREENINDEX);
        drawString4(90, 50, "START: Return to menu", GREENINDEX);
        if(BUTTON_PRESSED(BUTTON_START))
	{
            initialize();
	}
        flipPage();

}

void lose()
{
        fillScreen4(BLACKINDEX);
        drawString4(50,50, "BIRD JESUS WAS RELEASED", REDINDEX);
        drawString4(70,50, "THE DOME HAS WON", REDINDEX);
        drawString4(90, 50, "ALSO, GARY IS COOLER THAN YOU", REDINDEX);
        drawString4(110, 50, "START: Return to menu", REDINDEX);
        if(BUTTON_PRESSED(BUTTON_START))
	{
            initialize();
	}
        flipPage();
}

void up()
{   if(collisionBitmap[OFFSET(redSprite.bigRow+4-redSprite.rdel, redSprite.bigCol, 256)] == BLACK)
    {
        playSoundB(collide, COLLIDELEN, COLLIDEFREQ, 0);
    }
    else if(vOffZero > 0)
    {
        if (redSprite.row <= 80)
        {
            vOffZero-= redSprite.rdel;
        }
        else
        {
            redSprite.row-= redSprite.rdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);
    }
    else
    {
        
        if(redSprite.row > 0)
        {
             redSprite.row-= redSprite.rdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);

    }
    
}

void down()
{
    if(collisionBitmap[OFFSET(redSprite.bigRow+4+redSprite.rdel, redSprite.bigCol, 256)] == BLACK)
    {
        playSoundB(collide, COLLIDELEN, COLLIDEFREQ, 0);
    }
    else if(vOffZero < 256-160)
    {
        if (redSprite.row >= 70)
        {
            vOffZero+= redSprite.rdel;
        }
        else
        {
            redSprite.row+= redSprite.rdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);
    }
    else
    {
        if(redSprite.row < 160 - redSprite.height - 4)
        {
             redSprite.row+= redSprite.rdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);

    }
    
}

void left()
{   
    if(collisionBitmap[OFFSET(redSprite.bigRow+4, redSprite.bigCol - redSprite.cdel, 256)] == BLACK)
    {
        playSoundB(collide, COLLIDELEN, COLLIDEFREQ, 0);
    }
    else if(hOffZero > 0)
    {
        if (redSprite.col <= 120)
        {
            hOffZero-= redSprite.cdel;
        }
        else
        {
            redSprite.col-= redSprite.cdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);
    }
    else
    {
        if(redSprite.col > 0)
        {
             redSprite.col-= redSprite.cdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);

    }
    
}

void right()
{
    if(collisionBitmap[OFFSET(redSprite.bigRow+4, redSprite.bigCol + redSprite.cdel, 256)] == BLACK)
    {
        playSoundB(collide, COLLIDELEN, COLLIDEFREQ, 0);
    }
    else if(hOffZero < 256 - 240)
    {
        if (redSprite.col >= 110)
        {
            hOffZero+= redSprite.cdel;
        }
        else
        {
            redSprite.col+= redSprite.cdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);
    }
    else
    {
        if(redSprite.col < 240 - redSprite.width)
        {
             redSprite.col+= redSprite.cdel;
        }
        playSoundB(walk, WALKLEN, WALKFREQ, 0);

    }
    
}

void initialize()
{
	cheat = 0;
        setupInterrupts();
	setupSounds();
        
        vbCountAnimate = 0;
        animateCount = 0;
        
        redSprite.row = 28;
        redSprite.col = 32;
        redSprite.bigCol = 0;
        redSprite.bigRow = 0;
        redSprite.start = SPRITEOFFSET16(0,0);
        redSprite.shape = ATTR0_SQUARE;
        redSprite.size = ATTR1_SIZE16;
        redSprite.height = 16;
        redSprite.width = 16;
        redSprite.rdel = 16;
        redSprite.cdel = 16;
        
        arrow.row = 0;
        arrow.col = 0;
        arrow.bigCol = 0;
        arrow.bigRow = 0;
        arrow.start = SPRITEOFFSET16(0,8);
        arrow.shape = ATTR0_SQUARE;
        arrow.size = ATTR1_SIZE16;
        arrow.height = 16;
        arrow.width = 16;
        arrow.rdel = 0;
        arrow.cdel = 0;
        for (i = 0; i<NUMCOMPUTERS; i++)
        {
            computers[i].start = SPRITEOFFSET16(0,2);
            computers[i].shape = ATTR0_TALL;
            computers[i].size = ATTR1_SIZE32;
            computers[i].height = 32;
            computers[i].width = 16;
            computers[i].rdel = 16;
            computers[i].cdel = 16;
        }
        computers[0].bigRow = 13*16;
        computers[0].bigCol = 7*16;
        computers[1].bigRow = 12*16;
        computers[1].bigCol = 2*16;
        computers[2].bigRow = 3*16;
        computers[2].bigCol = 12*16;
        computers[3].bigRow = 7*16;
        computers[3].bigCol = 6*16;
        computers[4].bigRow = 9*16;
        computers[4].bigCol = 10*16;
        computers[5].bigRow = 4*16;
        computers[5].bigCol = 2*16;
        
        
        hOffZero = 0;
	vOffZero = 0;
        hOffOne = 0;
	state = STARTSCREEN;
        playSoundA(opening2, OPENING2LEN, OPENING2FREQ, 1);
	
}
