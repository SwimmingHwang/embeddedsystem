#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<time.h>
#include <linux/input.h>
#include<dirent.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include<sys/ipc.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
//#include <csignal>

#include "fbtobmp.h"

#define MAX_BUF 32
#define WDIR "/wav/"
#define BDIR "/bmp/"
#define EVENT_DEVICE	"/dev/input/event1"
#define EVENT_TYPE  	EV_ABS
#define EVENT_CODE_X	53
#define EVENT_CODE_Y	54
#define MAX_BUF_S 16	// buffer_size define for each clcd line 
#define MAX_BUF_B 32	// buffer_size define for total clcd
#define MAX_TITLELENGTH 16	// buffer_size define for movie title
//#define SIGUSER1 10
//#define SIGUSER2 12

int clcdFd, fndFd, ledFd;
int pid, volume, tspid;
int sig = 0;
//int *calC_num=0;
int default_vol = 20;
char movie[16];
char countNum[5][9] = {"one.wav","two.wav","three.wav","four.wav","five.wav" };
char countSec[10][6] = { "1.wav","2.wav","3.wav","4.wav","5.wav","6.wav","7.wav","8.wav","9.wav","10.wav" };
char bmp_name[10][15] = {"upLogo.bmp","armR_n.bmp","cobra_n.bmp","stretch_n.bmp","lunge_n.bmp","pushup_n.bmp","jump_n.bmp","power.bmp","minus.bmp","plus.bmp"};
int bmp_pos[10][2] = { {0,0}, {0,50}, {170,50}, {341,50}, {0,275}, {170,275}, {341,275},{0,500},{128,500}, {320,500} };
int cPercent;
int sigNum[6]={14,13,12,11,10,15};
void SigHandler(int signo) {
	if (signo == SIGINT) {
		exit(0);
	}
}
//
//	if (signo == SIGSEGV){
//		printf("\nSIGSEGV\n");
//		*calC_num=1;
//	}
//	// action1
//	
//	
//
//	if (signo == SIGUSER1){
//		printf("\nSIGUSER1\n");
//		*calC_num=2;
//	}
//
//	if (signo == SIGUSER2){
//		printf("\nSIGUSER2\n");
//		*calC_num=3;
//	}
//
//	if (signo == SIGALRM){
//		printf("\nSIGALRM\n");		
//		*calC_num=4;
//	}
//
//	if (signo == SIGPIPE){
//		printf("\nSIGPIPE\n");
//		*calC_num=5;
//	}
//
//	if (signo == SIGTERM){
//		printf("\nSIGTERM\n");
//		*calC_num=6;
//	}
//}	

int play(char *string)
{
	char dir[30] = WDIR;
	char volumeBuf[4];
	sprintf(volumeBuf, "%d", volume);   //itoa() 

	char *argv[] = { "mplayer", "-volume", volumeBuf, "-slave", "-quiet", "-input", "file=/usr/bin/mplayerfifo", "-ao", "alsa:device=hw=1.0", "-vo", "fbdev:/dev/fb0", strcat(dir, string), (char *)0 };

	if ((pid = fork()) == 0) {
		if (execv("/usr/bin/mplayer", argv) < 0) {
			perror("execv");
			return 0;
		}
	}
	else {
		return pid;
	}
}	// play movie (read from directory in system memory)

int playOpenCv()
{
	//char dir[30];
	//char volumeBuf[4];
	//sprintf(volumeBuf, "%d", volume);   //itoa() 

	char *argv[] = { 0 };

	if ((pid = fork()) == 0) {
		if (execv("/root/A", argv) < 0) {
			perror("execv");
			return 0;
		}
	}
	else {
		return pid;
	}
}	// play movie (read from directory in system memory)


int drawBmp(char *string, int xpos, int ypos)
{
	int result;
	char dir[30] = BDIR;
	strcat(dir, string);
	result = draw(dir, xpos, ypos*1024);
	printf("\nopen : %s",string);
	printf("\nxpos : %d, ypos : %d \n", xpos, ypos);
	return result;
}

	/*touch position*/
int check_ts(int x1, int y1, int x2, int y2, int x, int y)
{
	if (x > x1 && x < x2 && y > y1 && y < y2)
		return 1;
	else
		return 0;
}	// chech touch screen’s position


int main() {

	struct input_event ev;
	int tsFd, fndFd, clcdFd, ev_size, key, size;
	char isPressed = 0;
	char clcd_buf[32];
	char fnd_buf[4];

	unsigned char led = 0b00000000;

	int i;
	int count_i = 0;

	int count_flag = 0;

	int xpos = 0;
	int ypos = 0;

	char cmd[50];
	char buf[50];
	char setvolcmd1[] = "echo \"set_property volume ";
	char setvolcmd2[] = "\" > /usr/bin/mplayerfifo";

	int result;

	/*value for shared memory(for ts read)*/
	int shmid, shmid2, shmid_flag;
	int *cal_num, *cal_num2, *cal_flag;
	void *shared_memory = (void *)0;
	void *shared_memory2 = (void *)0;
	void *shared_memory_flag = (void *)0;

	/*value for shared memory(for get value from opencv)*/
	int shmidC;
	int *calC_num;
	void *shared_memoryC = (void *)0;
	

	tsFd = open(EVENT_DEVICE, O_RDONLY);
	if (tsFd == -1) {
		printf("%s is not a vaild device\n", EVENT_DEVICE);
		return EXIT_FAILURE;
	}
	if ((fndFd = open("/dev/fpga_fnd", O_RDWR)) < 0) {
		printf("Device open error : /fpga_led\n");
		exit(-1);
	}
	if ((ledFd = open("/dev/fpga_led", O_WRONLY))<0)
	{
		printf("Device open error : /dev/fpga_led\n");
		exit(-1);
	}

	playOpenCv();

	/*draw first display*/
	for (i = 0; i < 10;i++) {
		printf("\n%d inloop", i);
		result = drawBmp(bmp_name[i], bmp_pos[i][0], bmp_pos[i][1]);
		if (result) {
			perror("Draw Fail");
			printf("\n\nerror number : %d", i);
			exit(1);
		}
	}

	printf("\ndrawing done\n");

	volume = 30;
	play("opening.wav");

	//play("dont.mp3");
	
	//volume = default_vol;
	//printf("\n\nset volume : %d\n\n", volume);
	//sprintf(buf, "%d", volume);   //itoa() 
	//sprintf(cmd, "%s%s%s", setvolcmd1, buf, setvolcmd2);
	//system(cmd);
	//printf("\nafter volume line");

	//system("echo \"set_property volume 20\" > /usr/bin/mplayerfifo");

	system("echo \"set_property volume 30\" > /usr/bin/mplayerfifo");
//	printf("\nset volume\n");

	/*generate shared memory space*/
	shmid = shmget((key_t)1234, sizeof(int), 0666 | IPC_CREAT);
	shmid2 = shmget((key_t)5678, sizeof(int), 0666 | IPC_CREAT);
	shmid_flag = shmget((key_t)9012, sizeof(int), 0666 | IPC_CREAT);
	shmidC = shmget((key_t)1111, sizeof(int), 0666 | IPC_CREAT);
	if (shmid == -1 || shmid2 == -1 || shmid_flag == -1 || shmidC == -1)
	{
		perror("shmget failed : "); exit(0);
	}



	/*stick shared memory to process memory*/
	shared_memory = shmat(shmid, (void *)0, 0);
	shared_memory2 = shmat(shmid2, (void *)0, 0);
	shared_memory_flag = shmat(shmid_flag, (void*)0, 0);
	shared_memoryC = shmat(shmidC, (void *)0, 0);
	if (shared_memory == (void *)-1 || shared_memory2 == (void *)-1 || shared_memory_flag == (void*)-1 || shared_memoryC == (void*)-1)
	{
		perror("shmat failed: "), exit(0);
	}

	cal_num = (int *)shared_memory;
	cal_num2 = (int *)shared_memory2;
	cal_flag = (int*)shared_memory_flag;
	calC_num = (int *)shared_memoryC;

	
	printf("\nmake share memory done\n");
	

	tspid = fork();
	if (tspid == 0) {

	

		
		shmid = shmget((key_t)1234, sizeof(int), 0);
		shmid2 = shmget((key_t)5678, sizeof(int), 0);
		shmid_flag = shmget((key_t)9012, sizeof(int), 0);
		shmidC = shmget((key_t)1111, sizeof(int), 0);
		if (shmid == -1 || shmid2 == -1 || shmid_flag == -1 || shmidC == -1)
		{
			perror("shmget failed : "); exit(0);
		}
		shared_memory = shmat(shmid, (void *)0, 0666 | IPC_CREAT);
		shared_memory2 = shmat(shmid2, (void *)0, 0666 | IPC_CREAT);
		shared_memory_flag = shmat(shmid_flag, (void*)0, 0666 | IPC_CREAT);
		shared_memoryC = shmat(shmidC, (void *)0, 0666 | IPC_CREAT);


		if (shared_memory == (void *)-1 || shared_memory2 == (void *)-1 || shared_memory_flag == (void*)-1 || shared_memoryC == (void *)-1)
		{
			perror("shmat failed: "); exit(0);
		}

		cal_num = (int *)shared_memory;
		cal_num2 = (int *)shared_memory2;
		cal_flag = (int *)shared_memory_flag;
		calC_num = (int *)shared_memoryC;	//for *calC_num

		//*calC_num = 4;

		/*clock_t clock_start = 0, clock_end = 0;
		clock_start = clock();*/
				



		while (1) {


			/*if(*cal_flag == 1 && check_ts( 0, 0, 100, 100, *cal_num, *cal_num2)){
	play(countNum[0]);
	*cal_flag=0;
}*/

			if (clock() / CLOCKS_PER_SEC == 15)
			{
				if (*calC_num < 6) {
					*calC_num = 4;
				}
				//printf("\ncalC_num : %d\n", calC_num);
			}

			if (*cal_flag == 1 && check_ts(0, 500, 128, 600, *cal_num, *cal_num2)) {
				printf("bye~");
				sleep(1);
				kill(tspid, SIGINT);
				break;
			}//touch exit
			if (*cal_flag == 1 && check_ts(128, 500, 320, 600, *cal_num, *cal_num2)) {
				if(volume>0){volume-=5;}
				else{printf("volume=0");}
				printf("\nvol -\n");
				//system("echo \"volume 0\" > /usr/bin/mplayerfifo");
				//system("echo \"volume 0\" > /usr/bin/mplayerfifo");
				*cal_flag = 0;
			}//touch volume -
			if (*cal_flag == 1 && check_ts(320, 500, 512, 600, *cal_num, *cal_num2)) {
				if(volume<100){
					volume+=5;
				}
				else{printf("volume=100");}
				printf("\nvol +\n");
				//system("echo \"volume 1\" > /usr/bin/mplayerfifo");
				//system("echo \"volume 1\" > /usr/bin/mplayerfifo");
				*cal_flag = 0;
			}//touch volume +

			 //action 별 할일 fnd, led, audio
			 //횟수동작 횟수count
			if (*calC_num == 1) {
				//동작번호
				fnd_buf[0] = 0;
				fnd_buf[1] = 1;


				//일치율
				fnd_buf[2] = cPercent / 10;
				fnd_buf[3] = cPercent % 10;

				write(fndFd, &fnd_buf, 4);

				result=drawBmp("armR_o.bmp",0,50);
				if (result) {
					perror("arm.bmp Draw Fail");
					exit(1);
				}
				printf("\nprint armR_o\n");
				
				play(countNum[count_i]);
				count_i++;
				led = led >> 1;
				led = led + 0b10000000;
				write(ledFd, &led, sizeof(led));
				
				*calC_num =0;
			}
			if (*calC_num == 2) {
				//동작번호
				fnd_buf[0] = 0;
				fnd_buf[1] = 2;


				//일치율
				fnd_buf[2] = cPercent / 10;
				fnd_buf[3] = cPercent % 10;
				write(fndFd, &fnd_buf, 4);

				result=drawBmp("cobra_o.bmp",170,50);
				if (result) {
					perror("cobra.bmp Draw Fail");
					exit(1);
				}
								
				play(countNum[count_i]);
				count_i++;
				count_flag = 0;
				led = led >> 1;
				led = led + 0b10000000;
				write(ledFd, &led, sizeof(led));
				
				*calC_num =0;
			}
			if (*calC_num == 3) {
				//동작번호
				fnd_buf[0] = 0;
				fnd_buf[1] = 3;


				//일치율
				fnd_buf[2] = cPercent / 10;
				fnd_buf[3] = cPercent % 10;
				write(fndFd, &fnd_buf, 4);

				result=drawBmp("stretch_o.bmp", 341,50);
				if (result) {
					perror("stretch.bmp Draw Fail");
					exit(1);
				}
				
				play(countNum[count_i]);
				count_i++;
				count_flag = 0;
				led = led >> 1;
				led = led + 0b10000000;
				write(ledFd, &led, sizeof(led));
				
				*calC_num=0;
			}
			//시간동작 초count
			if (*calC_num == 4) {
				//동작번호
				fnd_buf[0] = 0;
				fnd_buf[1] = 4;


				//일치율
				fnd_buf[2] = cPercent / 10;
				fnd_buf[3] = cPercent % 10;
				write(fndFd, &fnd_buf, 4);

				result=drawBmp("pushup_o.bmp", 0,275);
				if (result) {
					perror("pushup.bmp Draw Fail");
					exit(1);
				}
				play("1sec.WAV");
				for (i = 0; i < 8; i++) {
					if (clock()% (CLOCKS_PER_SEC *4)== 0)
					{
						if (led == 0b11111111) {
							led = 0b00000000;
						}
						//play(countSec[0]);
						led = led >> 1;
						led = led + 0b10000000;
						write(ledFd, &led, sizeof(led));
						
					}
				}
				led = 0b00000000;
				*calC_num=0;

				result = drawBmp("pushup_n.bmp", 0, 275);
				if (result) {
					perror("pushup.bmp Draw Fail");
					exit(1);
				}
			}
			if (*calC_num == 5) {
				//동작번호
				fnd_buf[0] = 0;
				fnd_buf[1] = 5;


				//일치율
				fnd_buf[2] = cPercent / 10;
				fnd_buf[3] = cPercent % 10;
				write(fndFd, &fnd_buf, 4);

				result=drawBmp("lunge_o.bmp", 170, 275);
				if (result) {
					perror("lunge.bmp Draw Fail");
					exit(1);
				}
				for (i = 0; i < 10; i++) {
					if (clock() % 1000 == 0)
					{
						if (led == 0b11111111) {
							led = 0b00000000;
						}
						play(countSec[i]);
						led = led >> 1;
						led = led + 0b10000000;
						write(ledFd, &led, sizeof(led));
					}
				}
				led = 0b00000000;
				*calC_num=0;
			}
			if (*calC_num == 6) {
				//동작번호
				fnd_buf[0] = 0;
				fnd_buf[1] = 6;


				//일치율
				fnd_buf[2] = cPercent / 10;
				fnd_buf[3] = cPercent % 10;
				write(fndFd, &fnd_buf, 4);

				result=drawBmp("jump_o.bmp", 341, 275);
				if (result) {
					perror("jump.bmp Draw Fail");
					exit(1);
				}
				for (i = 0; i < 10; i++) {
					if (clock() % 1000 == 0)
					{
						if (led == 0b11111111) {
							led = 0b00000000;
						}
						play(countSec[i]);
						led = led >> 1;
						led = led + 0b10000000;
						write(ledFd, &led, sizeof(led));
					}
				}
				led = 0b00000000;
				*calC_num=0;
			}

		}

	}
	else if (tspid > 0) {
		//pid_2
		//touchscreen get position
		while (1)
		{


			ev_size = sizeof(ev);
			size = read(tsFd, &ev, ev_size);

			if (size < ev_size) {
				printf("Error size when reading\n");
				close(tsFd);
				return EXIT_FAILURE;
			}
			cal_flag = (int *)shared_memory_flag;

			if (ev.type == 1 && *cal_flag == 0)
			{
				*cal_flag = 1;
				*cal_num = 0;
				*cal_num2 = 0;
			}	// if cal_flag is 0 you can get input data (but if it’s 1 it means u r still touching the pad so block to get only one input value)
			if (ev.type == 3) {
				if (ev.code == EVENT_CODE_X) {
					xpos = ev.value;
					cal_num = (int *)shared_memory;
					*cal_num = xpos;
					printf("\nxpos : %d\n", xpos);
				}
				else if (ev.code == EVENT_CODE_Y) {
					ypos = ev.value;
					cal_num2 = (int*)shared_memory2;
					*cal_num2 = ypos;
					printf("ypos : %d\n", ypos);
				}
			}
		}

	}
	kill(pid, SIGINT);
	printf("Program Exit!\n");

	close(ledFd);
	close(tsFd);
	close(clcdFd);
	close(fndFd);

	return 0;
}
