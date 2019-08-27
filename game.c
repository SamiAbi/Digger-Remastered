/* game.c - xmain, prntr */

#include <conf.h>
#include <kernel.h>
#include <io.h>
#include <bios.h>
#include <math.h>
#include <sem.h>
#include <q.h>
extern SYSCALL  sleept(int);
extern struct intmap far *sys_imp;
/*------------------------------------------------------------------------
 *  xmain  --  example of 2 processes executing the same code concurrently
 *------------------------------------------------------------------------
 */

#define BLACK 0
#define DIAMOND 111
#define MONEY 46
#define BROWN 96
#define COIN 14
#define ENEMY 12
#define CHERRY 193
#define RIGHT 1
#define LEFT 2
#define UP 3
#define DOWN 4
#define ON (1)
#define OFF (0)
#define Reverse(x) ((x==1)?2:(x==2)?1:(x==3)?4:(x==4)?3:0)
int freezeFlag=0;
int enemyCurrDir[15] = {LEFT};
int soundHertz=255;
int enemyNum=0;
int mfpid,enemypid,clockCounterpid, firepid,soundpid;
int gno_of_pids;
int moneyFallFlag=0;
int fireflag=0;
int purpleFlag=0;
int purpleSec=90;
int moneyFallPos;
int Pos;
int Score=0;
int att;
int enemyArr[15]={302,306,310,314,318,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int map1[4000];
int lastDir=0;
int bulletPid;
int fireCD = 0;//counter for fire();
int level1=1;
int level2=0;
int level3=0;
int collectedDiamonds=0;
int gameOverFlag=0;
int seconds=0;
int lives=3;
int soundFlag=0;
int cherryFlag = 0;
int cherrySec = 0;
/*------------------------------------------------
 ChangeSpeaker - Turn speaker on or off. */

 void ChangeSpeaker( status )
 int status;
 {
  int portval;
//   portval = inportb( 0x61 );

      portval = 0;
   asm {
        PUSH AX
        MOV AL,61h
        MOV byte ptr portval,AL
        POP AX
       }

    if ( status==ON )
     portval |= 0x03;
      else
       portval &=~ 0x03;
        // outportb( 0x61, portval );
        asm {
          PUSH AX
          MOV AX,portval
          OUT 61h,AL
          POP AX
        } // asm

	} /*--ChangeSpeaker( )----------*/

	void Sound( hertz )
	int hertz;
	{
	 unsigned divisor = 1193180L / hertz;

	  ChangeSpeaker( ON );

   //        outportb( 0x43, 0xB6 );
        asm {
          PUSH AX
          MOV AL,0B6h
          OUT 43h,AL
          POP AX
        } // asm


     //       outportb( 0x42, divisor & 0xFF ) ;
        asm {
          PUSH AX
          MOV AX,divisor
          AND AX,0FFh
          OUT 42h,AL
          POP AX
        } // asm


     //        outportb( 0x42, divisor >> 8 ) ;

        asm {
          PUSH AX
          MOV AX,divisor
          MOV AL,AH
          OUT 42h,AL
          POP AX
        } // asm

	     } /*--Sound( )-----*/

      void NoSound( void )
        {
             ChangeSpeaker( OFF );
        } /*--NoSound( )------*/
/*this function draws a char on the screen the variable diff shows if to draw up down left right */
void Draw(times,spaces,diff,ch)
int times,spaces,diff;
char ch;
{
	int i=0,j=0,temp;
	while(i<times){
		//Draw 4 of the "-"
		asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,Pos
			MOV 			 AL,ch
			MOV              AH,00000000b	//Green background and intense red color
			MOV              CX,spaces       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
		for(j=0,temp=Pos;j<spaces*2;j++,temp+=1)
			map1[temp]=BLACK;
		i++;
		Pos+=diff;
	}
}
/****function for initializing display using cursor movements*****/
void setCursorPos(curPos)
int curPos;
{
	int row,col;
	char c,r;
	row=curPos/160;
	col=curPos%160;
	col/=2;
	c=(char)col;
	r=(char)row;
	asm{
		PUSH BX
		PUSH DX
		PUSH AX
		MOV AH,2
		MOV AL,0
		MOV BX,0
		MOV DH,r
		MOV DL,c
		INT 10H
		POP AX
	  POP DX
	  POP BX
	}
}
/*This function draw cherry after enemy dies and update map1*/
void DrawCherry(cherryPos)
int cherryPos;
{
		asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,cherryPos
			MOV 			 AL,'&'
			MOV              AH,11000001b	//Green background and intense red color
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
		map1[cherryPos] = CHERRY;
}
/*This function draw bullet "X"*/
void Drawbullet(bulletPos)
int bulletPos;
{
	asm{
		PUSH 			 BX
		PUSH  			 AX
		PUSH   		     DI
		MOV				 DI,bulletPos
		MOV 			 AL,'X'
		MOV              AH,001111b	//Green background and intense red color
		MOV              CX,1       	//; Initialize count, 1 Screen
		CLD                            //; Write forward
		REP              STOSW        // ; Write
		POP 			 DI
		POP 			 AX
		POP 			 BX
	}
}
/*This function called when game is over */
void gameOver()
{
	Pos=0;
	gameOverFlag=1;
	NoSound();
	soundFlag=0;
	Draw(4000,2,2,' ');
    asm{
        PUSH              BX
        PUSH               AX
        PUSH                DI
        MOV                 DI,1992
        MOV              AL,'G'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV            	DI,1994
        MOV              AL,'A'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,1996
        MOV              AL,'M'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,1998
        MOV              AL,'E'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,2000
        MOV              AL,' '
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,2002
        MOV              AL,'O'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,2004
        MOV              AL,'V'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,2006
        MOV              AL,'E'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,2008
        MOV              AL,'R'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	kill(clockCounterpid);
	kill(enemypid);
	kill(mfpid);
	kill(firepid);
}

/*This Function update score in screen*/
void updateScore(score)
int score;
{
	int sPos=20,temp;
	char c;
	while (score>0){
		temp=score%10;
		c=(char)('0'+temp);
		asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,sPos
			MOV 			 AL,c
			MOV              AH,00001110b	//Green background and intense red color
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
		sPos-=2;
		score/=10;
	}
}


/*this function draws a char on the screen the variable diff shows if to draw up down left right */
void DrawStar(starPos)
int starPos;
{
		asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,starPos
			MOV 			 AL,'*'
			MOV              AH,01101111b	//Green background and intense red color
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
		map1[starPos]=DIAMOND;
}
/* This function  Draw enemy in screen and update enemy in map1*/
void DrawEnemy(enemyPos)
int enemyPos;
{
    asm{
        PUSH              BX
        PUSH               AX
        PUSH                DI
        MOV                 DI,enemyPos
        MOV              AL,'#'
        MOV              AH,00001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	map1[enemyPos]=ENEMY;
}
void DrawEnemyPurple(enemyPos)
int enemyPos;
{
    asm{
        PUSH              BX
        PUSH               AX
        PUSH                DI
        MOV                 DI,enemyPos
        MOV              AL,'#'
        MOV              AH,10001001b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	map1[enemyPos]=ENEMY;
}
void DrawEnemyYellow(enemyPos)
int enemyPos;
{
    asm{
        PUSH              BX
        PUSH               AX
        PUSH                DI
        MOV                 DI,enemyPos
        MOV              AL,'#'
        MOV              AH,10001101b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	map1[enemyPos]=ENEMY;
}
void DrawMoney(moneyPos)
int moneyPos;
{
    asm{
        PUSH              BX
        PUSH               AX
        PUSH                DI
        MOV                 DI,moneyPos
        MOV              AL,'$'
        MOV              AH,00101110b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	map1[moneyPos]=MONEY;
}
void DrawMoneyWithoutBackground(moneyPos)
int moneyPos;
{
	int i;
	asm{
			PUSH              BX
			PUSH               AX
			PUSH                DI
			MOV                 DI,moneyPos
			MOV              AL,'$'
			MOV              AH,1110b    //Green background and intense red color
			MOV              CX,1           //; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP              DI
			POP              AX
			POP              BX
		}
		map1[moneyPos]=COIN;
		moneyPos+=2;
}
/*this function creates the main map */
void initDigerLevel3(){
    Pos=160;
    Draw(158,1,2,' ');
    Draw(5,1,160,' ');
    Draw(158,1,-2,' ');
    Draw(5,1,160,' ');
	Draw(158,1,2,' ');
    Draw(5,1,160,' ');
    Draw(158,1,-2,' ');
    Draw(5,1,160,' ');
	Draw(158,1,2,' ');
    Draw(5,1,160,' ');
    Draw(158,1,-2,' ');
    Draw(5,1,160,' ');
	
    Pos=240;
    asm{
        PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'@'
        MOV              AH,10001111b    //Green background and intense red color
        MOV              DI,Pos
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
    setCursorPos(Pos);
}

void initScreenLevel3(){
    int i,ep = 282;
	purpleSec=60;
	asm{
	  MOV              AH,0         
	  MOV              AL,3         
	  INT              10h           //; Adapter initialized. Page 0 displayed

	  MOV              AX,0B800h     //; Segment address of memory on color adapter

	  MOV              ES,AX         //; Set up extra segment register
	  MOV              DI,0          //; Initial offset address into segment
	  MOV              AL,' '        //; Character space to fill adapter memory
	  MOV              AH,01100000b      // ; Attribute byte : Intense yellow
	  MOV              CX,2000       //; Initialize count, 1 Screen
	  CLD                           // ; Write forward
	  REP              STOSW        // ; Write
	}
	for(i=0;i<4000;i++){
		map1[i]=BROWN;
	}
	DrawStar(100+i*2);
	for(i=0;i<78;i++)
	{
	    
        DrawStar(480+i*2);
        DrawStar(640+i*2);
		DrawStar(800+i*2);
	}
	for(i=0;i<78;i++)
	{
		DrawStar(1602+i*2);
        DrawStar(1282+i*2);
        DrawStar(1442+i*2);
	}
	for(i=0;i<78;i++)
	{
	    DrawStar(2400+i*2);
        DrawStar(2240+i*2);
        DrawStar(2080+i*2);
	}
	for(i=0;i<78;i++)
	{
        DrawStar(3042+i*2);
        DrawStar(3202+i*2);
		DrawStar(2882+i*2);
	}

    initDigerLevel3();
    DrawEnemy(318);
    DrawEnemy(314);
    DrawEnemy(310);
    DrawEnemy(306);
    DrawEnemy(302);
	DrawEnemy(298);
	DrawEnemy(294);
	DrawEnemy(290);
	DrawEnemy(286);
	DrawEnemy(282);


	for (i = 0; i<10; i++, ep += 4)
		enemyArr[i] = ep;
	for (i = 0; i<10; i++)
		enemyCurrDir[i] = LEFT;
	enemyNum = 15;

	setCursorPos(0);
	asm{
        PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'S'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,0
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'c'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,2
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'o'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,4
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'r'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,6
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'e'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,8
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,':'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,10
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,' '
        MOV              AH,00000000b    //Green background and intense red color
        MOV              DI,12
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
          //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
}


/*this function creates the main map */
void initDigerLevel2(){
    int newPos,Rectpos;
	purpleSec=60;
    Pos=160;
    Draw(20,2,2,' ');
	//Pos=240;
    Draw(20,1,160,' ');
    Draw(10,2,2,' ');
	Rectpos = Pos;
	Draw(15, 1,-160, ' ');
	Draw(5, 2, 2, ' ');
	Draw(5, 1, -160, ' ');
	Draw(25, 2, 2, ' ');
	newPos = Pos;
	Draw(19, 2, 2, ' ');
	Pos = newPos;
	Draw(10, 1, 160, ' ');
	Draw(30, 2, -2, ' ');
	Pos = Rectpos;
	Draw(25, 2, 2, ' ');
	Draw(10,1, -160, ' ');
	Draw(2, 5, 2, ' ');
	Draw(10, 1, -160, ' ');
	Pos = 180;
	Draw(10, 1, 160, ' ');
	Draw(3, 2, 2, ' ');
	Draw(3, 1, 160, ' ');
	Draw(5, 1, -2, ' ');
	Draw(10, 1, 160, ' ');
	Draw(22, 2, 2, ' ');
	Draw(4, 1,-160, ' ');

    Pos=3900;
    asm{
        PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'@'
        MOV              AH,10001111b    //Green background and intense red color
        MOV              DI,Pos
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
    setCursorPos(Pos);
}

/*this function paints the whole screen brown*/
void initScreenLevel2(){
    int i,ep = 302;
	purpleSec=60;
	asm{
	  MOV              AH,0         
	  MOV              AL,3         
	  INT              10h           //; Adapter initialized. Page 0 displayed

	  MOV              AX,0B800h     //; Segment address of memory on color adapter

	  MOV              ES,AX         //; Set up extra segment register
	  MOV              DI,0          //; Initial offset address into segment
	  MOV              AL,' '        //; Character space to fill adapter memory
	  MOV              AH,01100000b      // ; Attribute byte : Intense yellow
	  MOV              CX,2000       //; Initialize count, 1 Screen
	  CLD                           // ; Write forward
	  REP              STOSW        // ; Write
	}
	for(i=0;i<4000;i++){
		map1[i]=BROWN;
	}
	for(i=0;i<5;i++)
	{
	    DrawStar(488+i*160);
        DrawStar(490+i*160);
        DrawStar(492+i*160);
	}

	for(i=0;i<6;i++)
    {
        DrawStar(580+i*160);
        DrawStar(582+i*160);
        DrawStar(586+i*160);
        DrawStar(588+i*160);
    }

	for(i=0;i<9;i++)
	{
		//DrawStar(1000+i*160);
		DrawStar(1002+i*160);
		DrawStar(1004+i*160);
		DrawStar(1006+i*160);
		DrawStar(1008+i*160);
	}
	for(i=0;i<5;i++)
	{
		DrawStar(2156+i*160);
		DrawStar(2158+i*160);
		DrawStar(2160+i*160);
		DrawStar(2162+i*160);
		DrawStar(2164+i*160);
	}
	for(i=0;i<5;i++)
	{
		DrawStar(2530+i*160);
		DrawStar(2532+i*160);
		DrawStar(2534+i*160);
		DrawStar(2536+i*160);
		DrawStar(2538+i*160);
	}
    initDigerLevel2();
    DrawEnemy(318);
    DrawEnemy(314);
    DrawEnemy(310);
    DrawEnemy(306);
    DrawEnemy(302);

	for (i = 0; i<5; i++, ep += 4)
		enemyArr[i] = ep;
	for (i = 0; i<5; i++)
		enemyCurrDir[i] = LEFT;
	enemyNum = 10;
	
	DrawMoney(584);
	DrawMoney(1018);
	DrawMoney(1998);
	DrawMoney(2332);
	DrawMoney(3564);
	DrawMoney(786);
	
	setCursorPos(0);
	asm{
        PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'S'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,0
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'c'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,2
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'o'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,4
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'r'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,6
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'e'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,8
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,':'
        MOV              AH,00001110b    //Green background and intense red color
        MOV              DI,10
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,' '
        MOV              AH,00000000b    //Green background and intense red color
        MOV              DI,12
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
          //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
}

/*This function called when game is over */
void win()
{
	Pos=0;
	gameOverFlag=1;
	NoSound();
	soundFlag=0;
	Draw(4000,2,2,' ');
    asm{
        PUSH              BX
        PUSH               AX
        PUSH                DI
        MOV                 DI,1992
        MOV              AL,'W'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV            	DI,1994
        MOV              AL,'I'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,1996
        MOV              AL,'N'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,1998
        MOV              AL,'N'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,2000
        MOV              AL,'E'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		        MOV                 DI,2002
        MOV              AL,'R'
        MOV              AH,10001100b    //Green background and intense red color
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	kill(clockCounterpid);
	kill(enemypid);
	kill(mfpid);
	kill(firepid);
}
/*function start next level */
void nextLevel(){
	int i;
	if(level3==1){
		win();
	}
	if(level2==1){
		Pos=0;
		cherryFlag=0;
		Draw(4000,2,2,' ');
		level1=0;
		level2=0;
		level3=1;
		collectedDiamonds=0;
		resume(create(initScreenLevel3, INITSTK, INITPRIO, "DISPLAYER", 0) );
	}
	if(level1==1){
		Pos=0;
		cherryFlag=0;
		Draw(4000,2,2,' ');
		level1=0;
		level2=1;
		collectedDiamonds=0;
		resume(create(initScreenLevel2, INITSTK, INITPRIO, "DISPLAYER", 0) );
	}

}

void diggerDie(){
	asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,Pos
			MOV 			 AL,' '
			MOV              AH,00000000b	//Green background and intense red color
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
	Pos=3900;
    asm{
        PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'@'
        MOV              AH,10001111b    //Green background and intense red color
        MOV              DI,Pos
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	asm{
		 PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,' '
        MOV              AH,01100000b    //Green background and intense red color
        MOV              DI,150
        ADD              BX,BX
        MOV              CX,3           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		POP              DI
        POP              AX
        POP              BX
	}
	asm{
		 PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'@'
        MOV              AH,01101101b    //Green background and intense red color
        MOV              DI,150
        ADD              BX,BX
        MOV              CX,lives           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		POP              DI
        POP              AX
        POP              BX
	}

}

void moveEnemyLeft(int p){
	int tempPos=150,i;
	if(freezeFlag==0){
		if(cherryFlag==0){
			tempPos+=(2*lives);
			if(p-2==Pos){
				lives--;
				soundHertz=555;
				soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					diggerDie();
			}

				asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,p
					MOV 			 AL,' '
					MOV              AH,00000000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
				map1[p]=BLACK;
				if(purpleFlag==0)
					DrawEnemy(p-2);
				else
					DrawEnemyPurple(p-2);
		}
		if(cherryFlag==1){
			if(p-2==Pos){
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == (Pos-2))
					{
						tempPos=Pos-2;
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,tempPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						map1[tempPos]=BLACK;
						break;
					}

				}
			}
			else{
			asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,p
					MOV 			 AL,' '
					MOV              AH,00000000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
				map1[p]=BLACK;
				DrawEnemyYellow(p-2);
			}
		}
	}
}
void moveEnemyRight(int p){
	int tempPos=150,i;
	if(freezeFlag==0){
		tempPos+=(2*lives);
		if(cherryFlag==0){
			if(p+2==Pos){
				lives--;
				soundHertz=500;
				soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					diggerDie();
			}
				asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,p
					MOV 			 AL,' '
					MOV              AH,00000000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
				map1[p]=BLACK;
				if(purpleFlag==0)
					DrawEnemy(p+2);
				else
					DrawEnemyPurple(p+2);
		}
		if(cherryFlag==1){
			if(p+2==Pos){
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == (Pos+2))
					{
						tempPos=Pos+2;
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,tempPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						map1[tempPos]=BLACK;
						break;
					}

				}
			}
			else{
				asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,p
					MOV 			 AL,' '
					MOV              AH,00000000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
				map1[p]=BLACK;
				DrawEnemyYellow(p+2);
			}
		}
	}
}
void moveEnemyUp(int p){
	int tempPos=150,i;
	if(freezeFlag==0){
		tempPos+=(2*lives);
		if(cherryFlag==0){
			if(p-160==Pos){
				lives--;
				soundHertz=500;
				soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					diggerDie();
			}
				asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,p
					MOV 			 AL,' '
					MOV              AH,00000000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
				map1[p]=BLACK;
				if(purpleFlag==0)
					DrawEnemy(p-160);
				else
					DrawEnemyPurple(p-160);
		}
			if(cherryFlag==1){
				if(p-160==Pos){
					for (i = 0; i<10; i++)
					{
						if (enemyArr[i] == (Pos-160))
						{
							tempPos=Pos-160;
							enemyNum--;
							map1[enemyArr[i]] = BLACK;
							if (enemyNum == 0)
							{
								nextLevel();
								break;
							}
							else if (enemyNum<5)
								enemyArr[i] = -1;
							else
								enemyArr[i] = 318;

							asm{
								PUSH              BX
								PUSH               AX
								PUSH                DI
								MOV                 DI,tempPos
								MOV              AL,' '
								MOV              AH,00000000b    //Green background and intense red color
								MOV              CX,1           //; Initialize count, 1 Screen
								CLD                            //; Write forward
								REP              STOSW        // ; Write
								pop				DI
								pop				AX
								pop				BX
							}
							map1[tempPos]=BLACK;
							break;
						}

					}
				}
				else{
					asm{
						PUSH 			 BX
						PUSH  			 AX
						PUSH   		     DI
						MOV				 DI,p
						MOV 			 AL,' '
						MOV              AH,00000000b	//Green background and intense red color
						MOV              CX,1       	//; Initialize count, 1 Screen
						CLD                            //; Write forward
						REP              STOSW        // ; Write
						POP 			 DI
						POP 			 AX
						POP 			 BX
					}
					map1[p]=BLACK;
					DrawEnemyYellow(p-160);
				}
			}
	}
}
void moveEnemyDown(int p){
	int tempPos=150,i;
	if(freezeFlag==0){
		tempPos+=(2*lives);
		if(cherryFlag==0){
			if(p+160==Pos){
				lives--;
				soundHertz=500;
				soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					diggerDie();
			}
				asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,p
					MOV 			 AL,' '
					MOV              AH,00000000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
				map1[p]=BLACK;
				if(purpleFlag==0)
					DrawEnemy(p+160);
				else
					DrawEnemyPurple(p+160);
		}
		if(cherryFlag==1){
			if(p+160==Pos){
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == (Pos+160))
					{
						tempPos=Pos+160;
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,tempPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						map1[tempPos]=BLACK;
						break;
					}

				}
			}
			else{
				asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,p
					MOV 			 AL,' '
					MOV              AH,00000000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
				map1[p]=BLACK;
				DrawEnemyYellow(p+160);
			}
		}
	}
}

/*This function get  1 random diraction from 3 to move enemy */
int GetDir(i)
	int i;
	{
	int j=0,d,s=0,arr[3];
	arr[0]=arr[1]=arr[2]=0;
	if(purpleFlag==0){
		if(map1[enemyArr[i]-2]==BLACK && (enemyArr[i]-2)%160!=0 && Reverse(enemyCurrDir[i])!=LEFT){
			arr[j++]=LEFT;
			s++;
		}
		if(map1[enemyArr[i]+2]==BLACK && (enemyArr[i]+2)%160!=158 && Reverse(enemyCurrDir[i])!=RIGHT){
			arr[j++]=RIGHT;
			s++;
		}
		if(map1[enemyArr[i]-160]==BLACK && (enemyArr[i]-160)>158 && Reverse(enemyCurrDir[i])!=UP){
			arr[j++]=UP;
			s++;
		}
		if(map1[enemyArr[i]+160]==BLACK && (enemyArr[i]+160)<3998 && Reverse(enemyCurrDir[i])!=DOWN){
			arr[j++]=DOWN;
			s++;
		}
	}
	else{
		if(map1[enemyArr[i]-2]!=DIAMOND&&(enemyArr[i]-2)%160!=0 && Reverse(enemyCurrDir[i])!=LEFT){
			arr[j++]=LEFT;
			s++;
		}
		if(map1[enemyArr[i]+2]!=DIAMOND&&(enemyArr[i]+2)%160!=158 && Reverse(enemyCurrDir[i])!=RIGHT){
			arr[j++]=RIGHT;
			s++;
		}
		if(map1[enemyArr[i]-160]!=DIAMOND&&(enemyArr[i]-160)>158 && Reverse(enemyCurrDir[i])!=UP){
			arr[j++]=UP;
			s++;
		}
		if(map1[enemyArr[i]+160]!=DIAMOND&&(enemyArr[i]+160)<3998 && Reverse(enemyCurrDir[i])!=DOWN){
			arr[j++]=DOWN;
			s++;
		}
	}
	if(s==0)
	{
		d=Reverse(enemyCurrDir[i]);
		return d;
	}
	else 
	{
		d=rand()%s;
		return arr[d];
	}
}

/*this finction to moving all enemy (enemy process)*/
void startEnemies(){
	int i,dir;
	while(1){
		if(freezeFlag==0){
			receive();
			for(i=0;i<10;i++){
				if(enemyArr[i]!=-1){
					dir=GetDir(i);
					enemyCurrDir[i]=dir;
					if(dir==LEFT){
							moveEnemyLeft(enemyArr[i]);
							enemyArr[i]-=2;		
					}
					else if(dir==RIGHT){
							moveEnemyRight(enemyArr[i]);
							enemyArr[i]+=2;
					}
					else if(dir==UP){
								moveEnemyUp(enemyArr[i]);
								enemyArr[i]-=160;
					}
					else if(dir==DOWN){
								moveEnemyDown(enemyArr[i]);
								enemyArr[i]+=160;
					}
				}
			}
		}
		else{
			for(i=0;i<10;i++){
				enemyArr[i]=-1;
			}
		}
	}
}


void moveMoneyRight(){
	int moneyPos=(Pos+2);
	if (map1[moneyPos+2] != DIAMOND) {
		asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,moneyPos
			MOV 			 AL,' '
			MOV              AH,01100000b	//Green background and intense red color
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
		map1[moneyPos] = BROWN;
		//here we check is we are going right and the moneybag is above us and makes it fall
		DrawMoney(Pos + 4);
		if (map1[Pos + 164] != BROWN) {
			moneyFallPos = Pos + 164;
			moneyFallFlag = 1;
		}
	}
}
void moveMoneyLeft(){
	int moneyPos=(Pos-2);
	if (map1[moneyPos-2] != DIAMOND) {
		asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,moneyPos
			MOV 			 AL,' '
			MOV              AH,01100000b	//Green background and intense red color
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
		map1[moneyPos] = BROWN;
		DrawMoney(Pos - 4);
		if (map1[Pos + 156] != BROWN) {
			moneyFallPos = Pos + 156;
			moneyFallFlag = 1;
		}
	}
}

int sched_arr_pid[5] = {-1};//array for process ids to use in clkint.c (clock interrput)
int point_in_cycle[5] = {-1};//start point for all process
int gcycle_length[5]={-1};//cycle that all process haave to reached
int counterSecondpid;//id for proccres that count second in game

/*This function update seconds in screen*/
void updateSeconds(sec)
int sec;
{
	int sPos=60,temp;
	char c;
	while (sec>0){
		temp=sec%10;
		c=(char)('0'+temp);
		asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV				 DI,sPos
			MOV 			 AL,c
			MOV              AH,00001110b	//Green background and intense red color
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
		sPos-=2;
		sec/=10;
	}
}
/*this process count seconds in game  and update the seconds and reduce CDR for many FLAGS*/
void ClockCounterP(){
	
	while(1){
		receive();
		seconds++;
		updateSeconds(seconds);
		if(fireCD>0)
			fireCD--;
		purpleSec--;

		if(cherrySec>0)
			cherrySec--;

		if (cherrySec == 0) {
			cherryFlag=0;
		}
		if (soundFlag)
			send(soundpid, 1);
		if(purpleSec==0)
		{
			soundHertz=300;
			soundFlag=1;
			purpleFlag=1;
		}
		if(level1==1){
			if(purpleSec==-15){
				purpleSec=90;
				purpleFlag=0;
			}
		}
		if(level2==1){
			if(purpleSec==-20){
				purpleSec=60;
				purpleFlag=0;
			}
		}
		if(level3==1){
			if(purpleSec==-30){
				purpleSec=60;
				purpleFlag=0;
			}
		}
		
	}
}

/*this function move mony bag to down (fall) and if bag fall more than 2 rows draw coins in screen*/
void moveMoneyDown() {
	int i = 0, moneyPos = (moneyFallPos - 160), bPos, count = 0;
	//in this while we check if the moneybag is about to fall down and it keeps it fallin until we meet a brown surface
	while (map1[moneyPos + 160] != BROWN && (moneyPos + 160) <= 3999&&map1[moneyPos+160]!=DIAMOND) {
		receive();
		if (moneyPos + 160 != Pos) {
			count++;
			bPos = moneyPos;
			moneyPos += 160;
			asm{
				PUSH 			 BX
				PUSH  			 AX
				PUSH   		     DI
				MOV				 DI,bPos
				MOV 			 AL,' '
				MOV              AH,00000000b	//Green background and intense red color
				MOV              CX,1       	//; Initialize count, 1 Screen
				CLD                            //; Write forward
				REP              STOSW        // ; Write
				POP 			 DI
				POP 			 AX
				POP 			 BX
			}
			map1[bPos] = BLACK;
			DrawMoney(moneyPos);
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == moneyPos)
					{
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,moneyPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						DrawCherry(moneyPos);
						break;
					}

				}
		}
		else {
			lives--;
				diggerDie();
		}
	}
	if (count>2) {
		DrawMoneyWithoutBackground(moneyPos);
	}

}
/* money bag falling process call for moveMoneyDown function */
void MoneyFall() {
	while (1) {
		if (moneyFallFlag) {
			moveMoneyDown();
			moneyFallFlag = 0;
		}
	}
}
/*this process draw a bullet and move it in screen and if the bullet hit a enemy,enemy dies and update map1 */
void fire()
{
	int bulletPos,i=0,stopFlag=0;
	while(1){
		if(fireflag){
			i=0;
			stopFlag=0;
				if(lastDir==RIGHT)
				{
					bulletPos=Pos+2;
					while((map1[bulletPos]==BLACK || map1[bulletPos]==ENEMY) && ((bulletPos)%160!=159) && (stopFlag==0))
					{
						
						Drawbullet(bulletPos);
						receive();
						if(map1[bulletPos]==ENEMY)
							for(i=0;i<10;i++)
							{
								if(enemyArr[i]==bulletPos)
								{
									enemyNum--;
									map1[enemyArr[i]]=BLACK;
									if(enemyNum==0)
									{
										nextLevel();
										break;
									}
									else if(enemyNum<5)
										enemyArr[i]=-1;
									else
										enemyArr[i]=318;
									
									asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
									}
									DrawCherry(bulletPos);
									stopFlag=1;
									
									break;
								}
									
							}
						DrawCherry(bulletPos);
						if (stopFlag != 1) {
							asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
							}
							map1[bulletPos]=BLACK;
						}
							bulletPos+=2;
							if(level1==1)
								fireCD=2;
							if(level2==1)
								fireCD=4;
							if(level3==1)
								fireCD=7;
					}
				}
				if(lastDir==LEFT)
				{
					bulletPos=Pos-2;
					while((map1[bulletPos]==BLACK || map1[bulletPos]==ENEMY) && ((bulletPos)%160!=0) && (stopFlag==0))
					{
						Drawbullet(bulletPos);
						receive();
						if(map1[bulletPos]==ENEMY)
							for(i=0;i<10;i++)
							{
								if(enemyArr[i]==bulletPos)
								{
									enemyNum--;
									map1[enemyArr[i]]=BLACK;
									if(enemyNum==0)
									{
										nextLevel();
										break;
									}
									else if(enemyNum<5)
										enemyArr[i]=-1;
									else
										enemyArr[i]=318;
									asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
									}
									
									stopFlag=1;
									
									break;
								}
									
							}
						DrawCherry(bulletPos);
						if (stopFlag != 1) {
							asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
							}
							map1[bulletPos]=BLACK;
						}
						bulletPos -= 2;
						if(level1==1)
								fireCD=2;
							if(level2==1)
								fireCD=4;
							if(level3==1)
								fireCD=7;
					}
				}
				if(lastDir==UP)
				{
								bulletPos=Pos-160;
					while((map1[bulletPos]==BLACK || map1[bulletPos]==ENEMY) && ((bulletPos)>159) && (stopFlag==0))
					{
						Drawbullet(bulletPos);
						receive();
						if(map1[bulletPos]==ENEMY)
							for(i=0;i<10;i++)
							{
								if(enemyArr[i]==bulletPos)
								{
									enemyNum--;
									map1[enemyArr[i]]=BLACK;
									if(enemyNum==0)
									{
										nextLevel();
										break;
									}
									else if(enemyNum<5)
										enemyArr[i]=-1;
									else
										enemyArr[i]=318;
									asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
									}
									
									stopFlag=1;
									
									break;
								}
									
							}
						DrawCherry(bulletPos);
						if (stopFlag != 1) {
							asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
							}
							map1[bulletPos]=BLACK;
						}
							bulletPos-=160;
							if(level1==1)
								fireCD=2;
							if(level2==1)
								fireCD=4;
							if(level3==1)
								fireCD=7;;
					
					}
				}
				if(lastDir==DOWN)
				{
								bulletPos=Pos+160;
					while((map1[bulletPos]==BLACK || map1[bulletPos]==ENEMY) && ((bulletPos)<3999) && (stopFlag==0))
					{
						Drawbullet(bulletPos);
						receive();
						if(map1[bulletPos]==ENEMY)
							for(i=0;i<10;i++)
							{
								if(enemyArr[i]==bulletPos)
								{
									enemyNum--;
									map1[enemyArr[i]]=BLACK;
									if(enemyNum==0)
									{
										nextLevel();
										break;
									}
									else if(enemyNum<5)
										enemyArr[i]=-1;
									else
										enemyArr[i]=318;
									asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
									}
									
									stopFlag=1;
									
									break;
								}
									
							}
						DrawCherry(bulletPos);
						if (stopFlag != 1) {
							asm{
									PUSH              BX
									PUSH               AX
									PUSH                DI
									MOV                 DI,bulletPos
									MOV              AL,' '
									MOV              AH,00000000b    //Green background and intense red color
									MOV              CX,1           //; Initialize count, 1 Screen
									CLD                            //; Write forward
									REP              STOSW        // ; Write
									pop				DI
									pop				AX
									pop				BX
							}
							map1[bulletPos]=BLACK;
						}
							bulletPos+=160;
							if(level1==1)
								fireCD=2;
							if(level2==1)
								fireCD=4;
							if(level3==1)
								fireCD=7;
					
					
					}
				}	
		fireflag=0;
		}
	}
}

/*digger moving :*/
void updatePlayerUp(){
int tempPos=150,i;
tempPos+=(2*lives);

	//checks the boundaries of the screen 
	if((Pos-160)>=159){
		lastDir=UP;
		if(map1[Pos-160]==ENEMY){//pos-160 = 1 row up
			if(cherryFlag==0){
				lives--;
				soundHertz=500;
				soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,tempPos
					MOV 			 AL,' '
					MOV              AH,01100000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
			}
			else{
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == (Pos-160))
					{
						tempPos=Pos-160;
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,tempPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						map1[tempPos]=BLACK;
					}

				}
			}
		}
		if(map1[Pos-160]==DIAMOND){
			Score+=15;
			collectedDiamonds++;
			if(level1==1){
				if(collectedDiamonds==68){
					nextLevel();
				}
			}
			if(level2==1){
				if(collectedDiamonds==125){
					nextLevel();
				}
			}
			if(level3==1){
				if(collectedDiamonds==948){
					win();
				}
			}
			updateScore(Score);
		}
		if(map1[Pos-160]==COIN){
			int n=(rand()%4)+2;
			Score+=n;
			updateScore(Score);
		}
		if(map1[Pos-160]==CHERRY){
			cherryFlag=1;
			if(level1==1)
				cherrySec=15;
			if(level2==1)
				cherrySec=10;
			if(level3==1)
				cherrySec=5;
		}
		//else if we are going up a way where there is a moneybag 
		else{
			if(map1[Pos-160]==MONEY){
				moneyFallPos=Pos;
				moneyFallFlag=1;
				return;
			}
		}
		Draw(1,1,-160,' ');
	 	asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV 			 AL,'@'
			MOV              AH,00001111b	//Green background and intense red color
			MOV				 DI,Pos
			ADD		 	     BX,BX
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}

	}
}
void updatePlayerDown(){
	int tempPos=150,i;
tempPos+=(2*lives);
	if((Pos+160)<=3999){
		lastDir=DOWN;
		if(map1[Pos+160]==ENEMY){//pos-160 = 1 row up
			if(cherryFlag==0){
				lives--;
				soundHertz=500;
			soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,tempPos
					MOV 			 AL,' '
					MOV              AH,01100000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
			}
			else{
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == (Pos+160))
					{
						tempPos=Pos+160;
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,tempPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						map1[tempPos]=BLACK;
					}

				}
			}
		}
		if(map1[Pos+160]==DIAMOND){
			Score+=15;
			collectedDiamonds++;
			if(level1==1){
				if(collectedDiamonds==68){
					nextLevel();
				}
			}
			if(level2==1){
				if(collectedDiamonds==125){
					nextLevel();
				}
			}
			if(level3==1){
				if(collectedDiamonds==948){
					win();
				}
			}
			updateScore(Score);
		}
		if(map1[Pos+160]==COIN){
			int n=(rand()%4)+2;
			Score+=n;
			updateScore(Score);
		}
		if(map1[Pos+160]==CHERRY){
			cherryFlag=1;
			if(level1==1)
				cherrySec=15;
			if(level2==1)
				cherrySec=10;
			if(level3==1)
				cherrySec=5;
		}
		//this makes the player stuck when he tries to move down while there is a moneybag in the way just like in the game
		else if(map1[Pos+160]==MONEY)
			return;
		if (map1[Pos - 160] == MONEY)
		{
			moneyFallPos = Pos;
			moneyFallFlag = 1;
		}
	Draw(1,1,160,' ');
	 	asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV 			 AL,'@'
			MOV              AH,00001111b	//Green background and intense red color
			MOV				 DI,Pos
			ADD		 	     BX,BX
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
	}
}
void updatePlayerRight(){
	int temp=Pos,tempAtt,i;
	int tempPos=150;
	tempPos+=(2*lives);
	if(((Pos)%160)!=158){
		lastDir=RIGHT;
		
		if(map1[Pos+2]==ENEMY){//pos-160 = 1 row up
			if(cherryFlag==0){
				lives--;
				soundHertz=500;
			soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,tempPos
					MOV 			 AL,' '
					MOV              AH,01100000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
			}
			else{
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == (Pos+2))
					{
						tempPos=Pos+2;
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,tempPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						map1[tempPos]=BLACK;
					}

				}
			}
		}
		if(map1[Pos+2]==DIAMOND)
		{
			Score+=15;
			collectedDiamonds++;
			if(level1==1){
				if(collectedDiamonds==68){
					nextLevel();
				}
			}
			if(level2==1){
				if(collectedDiamonds==125){
					nextLevel();
				}
			}
			if(level3==1){
				if(collectedDiamonds==948){
					win();
				}
			}
			updateScore(Score);
		}
		if(map1[Pos+2]==COIN){
			int n=(rand()%4)+2;
			Score+=n;
			updateScore(Score);
		}
		if(map1[Pos+2]==CHERRY){
			cherryFlag=1;
			if(level1==1)
				cherrySec=15;
			if(level2==1)
				cherrySec=10;
			if(level3==1)
				cherrySec=5;
		}
		else if (map1[Pos + 2] == MONEY && ((Pos + 2) % 160) != 158) {
			if (map1[Pos + 4] != DIAMOND)
				moveMoneyRight();
			else
				return;
		}
		else if(((Pos+2)%160)==158 && map1[Pos+2]==MONEY)
			return;
			//if we are moving right and above us is a money bag and we moved the moneybag falls because we digged the ground underneath it
			else{
				if(map1[Pos-160]==MONEY){
					moneyFallPos=Pos;
					moneyFallFlag=1;
					Pos=temp;
				}
				//return;
			}
	Draw(1,1,2,' ');
	 	asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV 			 AL,'@'
			MOV              AH,00001111b	//Green background and intense red color
			MOV				 DI,Pos
			ADD		 	     BX,BX
			MOV              CX,1     	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
	}
}
void updatePlayerLeft(){
	int temp=Pos,i;
	int tempPos=150;
tempPos+=(2*lives);
	if(((Pos)%160)!=0){
		lastDir=LEFT;
		
		if(map1[Pos-2]==ENEMY){//pos-160 = 1 row up
			if(cherryFlag==0){
				lives--;
				soundHertz=500;
			soundFlag=1;
				if(lives==0){
					gameOver();
				}
				else
					asm{
					PUSH 			 BX
					PUSH  			 AX
					PUSH   		     DI
					MOV				 DI,tempPos
					MOV 			 AL,' '
					MOV              AH,01100000b	//Green background and intense red color
					MOV              CX,1       	//; Initialize count, 1 Screen
					CLD                            //; Write forward
					REP              STOSW        // ; Write
					POP 			 DI
					POP 			 AX
					POP 			 BX
				}
			}
			else{
				for (i = 0; i<10; i++)
				{
					if (enemyArr[i] == (Pos-2))
					{
						tempPos=Pos-2;
						enemyNum--;
						map1[enemyArr[i]] = BLACK;
						if (enemyNum == 0)
						{
							nextLevel();
							break;
						}
						else if (enemyNum<5)
							enemyArr[i] = -1;
						else
							enemyArr[i] = 318;

						asm{
							PUSH              BX
							PUSH               AX
							PUSH                DI
							MOV                 DI,tempPos
							MOV              AL,' '
							MOV              AH,00000000b    //Green background and intense red color
							MOV              CX,1           //; Initialize count, 1 Screen
							CLD                            //; Write forward
							REP              STOSW        // ; Write
							pop				DI
							pop				AX
							pop				BX
						}
						map1[tempPos]=BLACK;
					}

				}
			}
		}
		if(map1[Pos-2]==COIN){
			int n=(rand()%4)+2;
			Score+=n;
			updateScore(Score);
		}
		if(map1[Pos-2]==CHERRY){
			cherryFlag=1;
			if(level1==1)
				cherrySec=15;
			if(level2==1)
				cherrySec=10;
			if(level3==1)
				cherrySec=5;
		}
			if(map1[Pos-2]==DIAMOND){
				setCursorPos(0);
				Score+=15;
				collectedDiamonds++;
				if(level1==1){
					if(collectedDiamonds==68){
						nextLevel();
					}
				}
				if(level2==1){
					if(collectedDiamonds==125){
						nextLevel();
					}
				}
				if(level3==1){
				if(collectedDiamonds==948){
					win();
				}
			}
					updateScore(Score);
			}
		
			else if (map1[Pos - 2] == MONEY && (((Pos - 2) % 160) != 0)) {
				if (map1[Pos - 4] != DIAMOND)
					moveMoneyLeft();
				else
					return;
			}
		else if((((Pos-2)%160)==0) && map1[Pos-2]==MONEY)
			return;
			//same here as is the moving right, if we are moving left and above us is a moneybag we make it fall
			else{
				if(map1[Pos-160]==MONEY){
					moneyFallPos=Pos;
					moneyFallFlag=1;
					Pos=temp;
				}
			}
		 Draw(1,1,-2,' ');
	 	asm{
			PUSH 			 BX
			PUSH  			 AX
			PUSH   		     DI
			MOV 			 AL,'@'
			MOV              AH,00001111b	//Green background and intense red color
			MOV				 DI,Pos
			ADD		 	     BX,BX
			MOV              CX,1       	//; Initialize count, 1 Screen
			CLD                            //; Write forward
			REP              STOSW        // ; Write
			POP 			 DI
			POP 			 AX
			POP 			 BX
		}
	}
}

/*new interrupt 9 */
INTPROC new_int9(int mdevno)
{
 int scan = 0;
  int ascii = 0;

asm {
  MOV AH,1
  INT 16h
  JZ Skip1
  MOV AH,0
  INT 16h
  MOV BYTE PTR scan,AH
  MOV BYTE PTR ascii,AL
 } //asm
 if(!gameOverFlag){
	if (scan == 72){
		updatePlayerUp();
	}
	else  if (scan == 80){
		updatePlayerDown();
	}
	else  if (scan == 77){
		updatePlayerRight();
	}
	else  if (scan == 75){
		updatePlayerLeft();
		
	}
		else  if (scan == 57){//space
		if(fireCD<=0){
			soundHertz=555;
			soundFlag=1;
			fireflag=1;
		}
	}
	else  if (scan == 49) {
		nextLevel();
	}

	else  if (scan == 34) {
			if(freezeFlag==0)
				freezeFlag=1;
			else
				freezeFlag=0;
	}
 }

 if ((scan == 46)&& (ascii == 3)) // Ctrl-C?
   asm INT 27; // terminate xinu


Skip1:
return;
} // new_int9
/*this function set new interrupt 9*/
void set_new_int9_newisr()
{
  int i;
  for(i=0; i < 32; i++)
    if (sys_imp[i].ivec == 9)
    {
     sys_imp[i].newisr = new_int9;
     return;
    }

} // set_new_int9_newisr

/*this function creates the main map */
void initDiger(){
    int newPos;
    Pos=160;
    Draw(20,2,2,' ');
    Draw(10,1,160,' ');
    Draw(20,2,2,' ');
    newPos=Pos;
    Pos-=20;
    Draw(10,1,160,' ');
    Pos-=10;
    Draw(4,12,160,' ');
    Pos=newPos;
    Draw(10,1,-160,' ');
    Draw(20,2,2,' ');
    Draw(5,1,160,' ');
    Draw(5,2,2);
    Draw(10,1,160,' ');
    newPos=Pos;
    Draw(4,12,160,' ');
    Pos=newPos-4;
    Pos+=320;
    Draw(34,2,-2,' ');
    Pos=240;
    Draw(39,2,2,' ');
    Pos=300;
    Draw(1,10,160,' ');

    Pos=3900;
    asm{
        PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'@'
        MOV              AH,10001111b    //Green background and intense red color
        MOV              DI,Pos
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
    setCursorPos(Pos);
}
/*this function paints the whole screen brown*/
void initScreen(){
    int i;
	enemyNum=8;
	asm{
	  MOV              AH,0         
	  MOV              AL,3         
	  INT              10h           //; Adapter initialized. Page 0 displayed

	  MOV              AX,0B800h     //; Segment address of memory on color adapter

	  MOV              ES,AX         //; Set up extra segment register
	  MOV              DI,0          //; Initial offset address into segment
	  MOV              AL,' '        //; Character space to fill adapter memory
	  MOV              AH,01100000b      // ; Attribute byte : Intense yellow
	  MOV              CX,2000       //; Initialize count, 1 Screen
	  CLD                           // ; Write forward
	  REP              STOSW        // ; Write
	}
	for(i=0;i<4000;i++){
		map1[i]=BROWN;
	}
	for(i=0;i<4;i++)
	{
	    DrawStar(350+i*160);
        DrawStar(352+i*160);
        DrawStar(354+i*160);
	}

	for(i=0;i<4;i++)
    {
        DrawStar(580+i*160);
        DrawStar(582+i*160);
        DrawStar(584+i*160);
        DrawStar(586+i*160);
    }

	for(i=0;i<8;i++)
	{
		DrawStar(1940+i*160);
		DrawStar(1942+i*160);
		DrawStar(1944+i*160);
		DrawStar(1946+i*160);
		DrawStar(1948+i*160);
	}
    initDiger();
    DrawEnemy(318);
    DrawEnemy(314);
    DrawEnemy(310);
    DrawEnemy(306);
    DrawEnemy(302);
	DrawMoney(480);
	DrawMoney(2648);
	DrawMoney(1588);
	DrawMoney(786);
	DrawMoney(996);
	DrawMoney(550);


	setCursorPos(0);
	asm{
        PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'S'
        MOV              AH,01101110b    //Green background and intense red color
        MOV              DI,0
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'c'
        MOV              AH,01101110b    //Green background and intense red color
        MOV              DI,2
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'o'
        MOV              AH,01101110b    //Green background and intense red color
        MOV              DI,4
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'r'
        MOV              AH,01101110b    //Green background and intense red color
        MOV              DI,6
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,'e'
        MOV              AH,01101110b    //Green background and intense red color
        MOV              DI,8
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,':'
        MOV              AH,01101110b    //Green background and intense red color
        MOV              DI,10
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		MOV              AL,' '
        MOV              AH,01100000b    //Green background and intense red color
        MOV              DI,12
        ADD              BX,BX
        MOV              CX,1           //; Initialize count, 1 Screen
        CLD                            //; Write forward
          //; Write forward
        REP              STOSW        // ; Write
        POP              DI
        POP              AX
        POP              BX
    }
	//setCursorPos(74);
	asm{
		 PUSH             BX
        PUSH             AX
        PUSH             DI
        MOV              AL,'@'
        MOV              AH,01101101b    //Green background and intense red color
        MOV              DI,150
        ADD              BX,BX
        MOV              CX,3           //; Initialize count, 1 Screen
        CLD                            //; Write forward
        REP              STOSW        // ; Write
		POP              DI
        POP              AX
        POP              BX
	}
} 		

/*this function set schedule btween procress in clock inturrpt*/
SYSCALL schedule(int no_of_pids, int pid1, ...)//... = number of procs unknown
/*schedule (number of processes to schedule, pid of proc,...,starting time, finishing time)*/
{
  int i;
  int ps;
  int *iptr;

  disable(ps);
  gno_of_pids = no_of_pids;

  iptr = &pid1;
  for(i=0; i < no_of_pids; i++)
  {
    sched_arr_pid[i] = *iptr;//pid
    iptr++;
    point_in_cycle[i] = *iptr;//starting time
    iptr++;
	gcycle_length[i] = *iptr;//finishing time
    iptr++;
  } // for
  counterSecondpid=*iptr;
  restore(ps);

} // schedule

//procress to hear sounds
void soundPro(){
	while(1){
		if(soundFlag){
			Sound(soundHertz);
			receive();
			NoSound();
			soundFlag=0;
		}
	}
}

xmain()
{
	
	resume( clockCounterpid = create(ClockCounterP, INITSTK, INITPRIO, "Timing", 0) );
	resume(create(initScreen, INITSTK, INITPRIO, "DISPLAYER", 0) );
	resume( mfpid = create(MoneyFall, INITSTK, INITPRIO, "Money Fall", 0) );
	resume( enemypid = create(startEnemies, INITSTK, INITPRIO, "Enemy Moving", 0) );
	resume( firepid = create(fire, INITSTK, INITPRIO, "Bullet Moving", 0) );
	resume( soundpid = create(soundPro, INITSTK, INITPRIO, "Sound", 0) );

	set_new_int9_newisr();
	schedule(3,mfpid,0,4,enemypid,0,4,firepid,0,2,clockCounterpid);

	return;
} // xmain
