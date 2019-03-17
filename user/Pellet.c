#include "OS.h"
#include "indicator.h"
#include "Pellet.h"

/**
*	map: x:21~31 y:0~20
*/

uint8_t xdata boll2str[4];
enum DIR{UP=1,DOWN,LEFT,RIGHT} DirX = RIGHT,DirY = DOWN;
uint8_t PelletCoorX = 22,PelletCoorY = 4;

uint8_t* Number2String_(uint16_t n)
{
	boll2str[0] = n/100 + '0';
	boll2str[1] = (n%100)/10 + '0';
	boll2str[2] = n%10 + '0';
	boll2str[3] = 0;
	return boll2str;
}

void DisplayPellet(uint8_t x,uint8_t y,uint8_t color)
{
	DisplaySendString_("cirs ");
	DisplaySendString_(Number2String_(x*10+6));
	DisplaySendString_(",");
	DisplaySendString_(Number2String_(y*10+6));
	DisplaySendString_(",6,");
	switch(color)
	{
		case 0: DisplaySendString_("64991");break;
		case 1: DisplaySendString_("BROWN");break;
		case 2: DisplaySendString_("21130");break;
	}
	DisplaySendEnd_();
}

void UserProcess_Pellet()
{
	while (signal == 0);
	while(1)
	{
		DelayBlock_Proccess(1000);
		SemaphoreWait(&screen);
		DisplayPellet(PelletCoorX,PelletCoorY,2);
		SemaphoreSignal(&screen);
		if (DirX == RIGHT)
		{
			if (PelletCoorX == 31) DirX = LEFT;
			else PelletCoorX++;
		}else{
			if (PelletCoorX == 21) DirX = RIGHT;
			else PelletCoorX--;
		}
		if (DirY == DOWN)
		{
			if (PelletCoorY == 19) DirY = UP;
			else PelletCoorY++;
		}else{
			if (PelletCoorY == 0) DirY = DOWN;
			else PelletCoorY--;
		}
		SemaphoreWait(&screen);
		DisplayPellet(PelletCoorX,PelletCoorY,0);
		SemaphoreSignal(&screen);
	}
}