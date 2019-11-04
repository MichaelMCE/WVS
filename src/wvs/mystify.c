/****************************
* Mysticfy
* Written by Hung Ki Chan
* One clear but cold day in April 2006
* Loosely based on the myLCD Ball example
****************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void mystifyUpdateNodes (TFRAME *frm);
void mystifyDrawLineConnects (TFRAME *frm, int mode);
int mystifyInit (TFRAME *frame, void *userPtr);
void mystifyCleanup ();
int renderMyst (TWINAMP *wa, TFRAME *frame, void *data);

static int *dx, *dy;
static int *px, *py;
static int ctrl;
static int mysInitOnce = 0;




int mystPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderMyst(wa, frame, userPtr);
}

int mystPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return -1;
}

int mystPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("mystPageEnter\n");
	return 1;
}

void mystPageExit (void *userPtr)
{
	//dbprintf("mystPageExit\n");
}

int mystPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("mystPageOpen()\n");
	return mystifyInit(frame, userPtr);
}

void mystPageClose (void *userPtr)
{
	//dbprintf("mystPageClose()\n");
	mystifyCleanup();

}

int mystifyInit (TFRAME *frame, void *userPtr)
{
	if (mysInitOnce)
		mystifyCleanup();
	
	TWINAMP *wa = (TWINAMP *)userPtr;
	ctrl = wa->config.mystifyNodes;
	dx = (int *)calloc(ctrl, sizeof(int));
	dy = (int *)calloc(ctrl, sizeof(int));
	px = (int *)calloc(ctrl, sizeof(int));
	py = (int *)calloc(ctrl, sizeof(int));
	if (!(dx&&dy&&px&&py)){
		return 0;
	}

	//randomize the start coord. for each node
	int i;
    for(i=ctrl; i--;){
		px[i]=rand()%frame->width;
		py[i]=rand()%frame->height;
		dx[i]=1;
		dy[i]=1;
    }
    
    i = rand()&0x7FFF;
    while (i--)
		mystifyUpdateNodes(frame);
	
	mystifyDrawLineConnects(frame, LSP_CLEAR);
	mystifyUpdateNodes(frame);
	mystifyDrawLineConnects(frame, LSP_SET);
	mysInitOnce = 1;
	return 1;
}

void mystifyCleanup ()
{
	free(dx); dx = NULL;
	free(dy); dy = NULL;
	free(px); px = NULL;
	free(py); py = NULL;
	mysInitOnce = 0;
}

void mystifyDrawLineConnects (TFRAME *frm, int mode)
{
	int i;
	for(i = ctrl; i--;){
		if(i==ctrl-1)
           	lDrawLine(frm, *(px+i), *(py+i), *px, *py, mode);
		else
			lDrawLine(frm, *(px+i), *(py+i), px[i+1], py[i+1], mode);
	}
}

//update all the nodes coords. adjust each node's speed
//important, dx and dy not =0 or u get strange things happen
void mystifyUpdateNodes (TFRAME *frm)
{
	int *x = px;
	int *y = py;
	
	int i;
	for(i=ctrl; i--;){
		x[i] += dx[i];
		y[i] += dy[i];
              
		if( x[i]< 0){
			x[i] = 0;
			dx[i]=1+(rand()&1);           //different speed, very important, so u dont get same shape
		}else if(x[i]> frm->width){
			x[i] = frm->width;
			dx[i]=-1*(1+(rand()&1));       //not so sure but works, yup this should be invert
		}
              
		if(y[i] < 0){
			y[i] = 0;
			dy[i]=1+(rand()&1);
		}else if(y[i]  > frm->height){
			y[i] = frm->height;
			dy[i]=-1*(1+(rand()&1));
		}
	}
}

int renderMyst (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (!mysInitOnce){
		mystifyInit(frame, data);
		if (!mysInitOnce)
			return 0;
	}

	mystifyDrawLineConnects(frame, LSP_CLEAR);
	mystifyUpdateNodes(frame);
	mystifyDrawLineConnects(frame, LSP_SET);
	//lDrawRectangle(frame, 0, 0, frame->width, frame->height, LSP_SET);
	return 1;
}


