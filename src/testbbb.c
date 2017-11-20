//MH****************** Copyright (c) by GC *****************************
// Filename 		: init1.c
// Owner			: Gigaset Communications Polska Sp. z o.o.
//-------------------------------------------------------------------------
// Function			: Initialization of BeagleBone Black's ports -gpio(62,63), pwm(42,22)
// Special cases	: Only for Debian Embedded version BeagleBone Black Board
//-------------------------------------------------------------------------
//*************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <poll.h>
#include <stropts.h>
#include <sys/types.h>

#include "led.c"
#include "bb_functions.c"

//***** Module Specific Defines and Enums *********************************
//***** Module Specific Macros and Typedefs *******************************
//***** Definition of Global Variables ************************************

#define L_PWM 		"pwm_test_P9_42.15"
#define R_PWM 		"pwm_test_P9_22.16"
#define L_GPIO 		"/sys/class/gpio/gpio49"

#define L_GPIO_1	"27"
#define L_GPIO_2	"46"
#define R_GPIO_1	"44"
#define R_GPIO_2	"45"

#define L_SPEED		"7500"
#define R_SPEED		"7500"

//***** Definition of Static Variables ************************************
//***** Prototypes of Static Functions ************************************

static void gpio_setvalue(char pin[], char *value);
static void pwm_setup(char pin[], char set[], char *value);
static void GPIOChange(int gpio);

static int getVoltage();

static void turnleft();
static void go();
static void stop();
static void turn_around();
static void backward();
static void right_forward(int dir);
static void left_forward(int dir);

static int move = 0; // 0 - stop, 1 - going backwards, 2 - going forwards
static int speed = 7500;

int main(void) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	int n = 0;
	unsigned char buf = '\0';
	int USB = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY /*| O_NDELAY*/);
	char comm[50];

	char spd[10] = L_SPEED;
	// LEDs
	const char *LEDB[4] = {
			"/sys/class/leds/beaglebone:green:usr0/brightness",
			"/sys/class/leds/beaglebone:green:usr1/brightness",
			"/sys/class/leds/beaglebone:green:usr2/brightness",
			"/sys/class/leds/beaglebone:green:usr3/brightness"
	 };
	FILE *LED[4] = {NULL,NULL,NULL,NULL};
	LEDChange(LED,LEDB,0);

	pwm_init("bone_pwm_P9_42");
	usleep(500000);

	pwm_init("bone_pwm_P9_22");
	usleep(500000);

	am33xx_pwm();
	usleep(500000);

	pwm_setup(L_PWM,"duty",L_SPEED);
	usleep(500000);
	pwm_setup(L_PWM,"period","10000");
	usleep(500000);
	pwm_setup(L_PWM,"run","0");
	usleep(500000);

	pwm_setup(R_PWM,"duty",R_SPEED);
	usleep(500000);
	pwm_setup(R_PWM,"period","10000");
	usleep(500000);
	pwm_setup(R_PWM,"run","0");
	usleep(500000);

	gpio_init(L_GPIO_1);
	gpio_output(L_GPIO_1);
	gpio_setvalue(L_GPIO_1,"1");

	gpio_init(L_GPIO_2);
	gpio_output(L_GPIO_2);
	gpio_setvalue(L_GPIO_2,"0");

	gpio_init(R_GPIO_1);
	gpio_output(R_GPIO_1);
	gpio_setvalue(R_GPIO_1,"1");

	gpio_init(R_GPIO_2);
	gpio_output(R_GPIO_2);
	gpio_setvalue(R_GPIO_2,"0");

	char response[1024];
	memset(response, '\0', sizeof response);

	if(USB == -1)
	{
		perror("Unable to open the port");
	}

	//***** Error handler *****************************************************
	if (tcgetattr (USB,&tty)!=0)
	{
		printf("Error %d from tcgetattr: %s \n",errno,strerror(errno));
	}
	//***** Save old tty parameters *******************************************

	cfsetospeed(&tty,(speed_t)B38400);
	cfsetispeed(&tty,(speed_t)B38400);
	//***** Setting other port stuff ******************************************

	tty.c_cflag 	&= 	~PARENB;
	tty.c_cflag 	&= 	~CSTOPB;
	tty.c_cflag 	&=	~CSIZE;
	tty.c_cflag		|=	CS8;

	tty.c_lflag 	&= ~ICANON;

	tty.c_cflag 	&=	~CRTSCTS;		//	no flow control
	tty.c_cc[VMIN]	= 	0;				//	read doesn't block
	tty.c_cc[VTIME] = 	1;				//	0.1 seconds read timeout
	tty.c_cflag		|=	CREAD | CLOCAL;	//	turn on READ & ignore ctrl lines


	//***** Make raw **********************************************************
	//cfmakeraw(&tty);

	//***** Flush Port, then applies attributes *******************************
	tcflush(USB, TCIFLUSH);
	if(tcsetattr (USB, TCSANOW, &tty) != 0)
	{
		printf("Error %d from tcsetattr \n", errno);
	}

	//***** Reading from serial port loop ************************************

	while(1)
	{
		n = read(USB, &buf, 1);
		sprintf(response, "%c", buf);

		printf("%s \n", response);

		if (n == -1) // read error - break loop
		{
			printf("Error reading from serial port: %s\n", strerror(errno));
			stop();
			break;
		}
		else if (n == 0) // no data - stop motors
		{
			printf("No data\n");
			stop();
			LEDChange(LED,LEDB,0);
		}
		else
		{
			buf = '\0';
			printf("\n");
			if(strncmp(response,"F",1)==0) // forward
			{
				LEDChange(LED,LEDB,1111);
				move = 2;
				go();
			}
			else if(strncmp(response,"B",1)==0) // backward
			{
				LEDChange(LED,LEDB,1001);
				move = 1;
				backward();
			}
			else if(strncmp(response,"R",1)==0) // right
			{
				LEDChange(LED,LEDB,11);
				if(move > 0) right_forward(move);
			}
			else if(strncmp(response,"L",1)==0) // left
			{
				LEDChange(LED,LEDB,1100);
				if(move > 0) left_forward(move);
			}
			else if(strncmp(response,"S",1)==0) // stop moving forw/backw
			{
				LEDChange(LED,LEDB,0);
				move = 0;
				stop();
			}
			else if(strncmp(response,"s",1)==0) // stop turning
			{
				pwm_setup(L_PWM,"duty",L_SPEED);
				pwm_setup(R_PWM,"duty",R_SPEED);
				speed = atoi(L_SPEED);
				if(move == 1) backward();
				else if(move == 2) go();
				else
				{
					LEDChange(LED,LEDB,0);
					move = 0;
					stop();
				}
			}
			else if(strncmp(response,"Z",1)==0) // stop & break loop
			{
				LEDChange(LED,LEDB,0);
				stop();
				break;
			}
			else if(strncmp(response,"H",1)==0) // turn around
			{
				LEDChange(LED,LEDB,0);
				turn_around();
			}
			else if(strncmp(response,"V",1)==0) // volume up
			{
				strcpy(comm, "amixer sset Speaker 10\%+");
				system(comm);

				strcpy(comm, "aplay /root/BTled/wav/up &");
				system(comm);
			}
			else if(strncmp(response,"v",1)==0) // volume down
			{
				strcpy(comm, "amixer sset Speaker 5\%-");
				system(comm);

				strcpy(comm, "aplay /root/BTled/wav/down &");
				system(comm);
			}
			else if(strncmp(response,"i",1)==0) // info
			{
				strcpy(comm, "amixer sset Speaker 5\%-");
				system(comm);
			}
			else if(strncmp(response,"x",1)==0) // info
			{
				strcpy(comm, "aplay /root/BTled/wav/battery");
				system(comm);

				sprintf(comm, "echo \" %d point %d volts\" | festival --tts", 3 + getVoltage()/1000, getVoltage()%1000);
				system(comm);
			}
			else if(strncmp(response,"f",1)==0) // add speed
			{
				if(speed > 0) speed -= 500;
				sprintf(spd, "%d", speed);

				pwm_setup(L_PWM,"duty",spd);
				pwm_setup(R_PWM,"duty",spd);
			}
			else if((strncmp(response,"\0",1)==0)||(strncmp(response,"\n",1)==0))
			{
				printf("last char of response ");
			}
			else
			{
				printf("%s", response);
			}
		}
	}

	gpio_uninit(L_GPIO_1);
	gpio_uninit(L_GPIO_2);
	gpio_uninit(R_GPIO_1);
	gpio_uninit(R_GPIO_2);
}

static int getVoltage()
{
	static char value[10];

	FILE *GPIOHandle = NULL;
	char GPIOPath[50] = "/sys/devices/ocp.3/helper.17";
	sprintf(GPIOPath, "%s/AIN6",GPIOPath);
	GPIOHandle = fopen(GPIOPath, "r");
	fscanf(GPIOHandle, "%s", value);
	fclose(GPIOHandle);

	return atoi(value);
}

//PH****************** Copyright (c) by GC *****************************
static void gpio_setvalue
(
char pin[], 			// number of GPIO pin for value set
char *value				// value
)
// * FUNCTION		: Setting value (1 or 0) for GPIO output configuration
// *				: (pin[] - e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: GPIO pin must be initialized first and set output configuration
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	if ((strncmp(value,"1",1)==0)||(strncmp(value,"0",1))==0)
	{
		FILE *GPIOHandle = NULL;
		char GPIOPath[50] = "/sys/class/gpio";
		sprintf(GPIOPath, "%s/gpio%s/value",GPIOPath, pin);

		GPIOHandle = fopen(GPIOPath, "w");
		fprintf(GPIOHandle, value);
		fclose(GPIOHandle);

		printf("GPIO_%s value=%s \n",pin,value);
	}
	else
		printf("Wrong value\n");
}

//PH****************** Copyright (c) by GC *****************************
static void pwm_setup
(
char pin[],			// pwm path f.e. bone_pwm_P9_22
char set[], 		// f.e duty, period, run
char *value			// value of setting
)
// * FUNCTION		: Setting value of pwm parameters
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *PWMHandle = NULL;
	char GPIOPath[80] = "/sys/devices/ocp.3";
	sprintf(GPIOPath, "%s/%s/%s",GPIOPath, pin, set);

	PWMHandle = fopen(GPIOPath, "w");
	fprintf(PWMHandle, value);
	fclose(PWMHandle);

	if (strncmp(pin,R_PWM,17)==0)
	printf("LEFT MOTOR %s=%s \n",set,value);

	if (strncmp(pin,L_PWM,17)==0)
	printf("RIGHT MOTOR %s=%s \n",set,value);
}
//PH****************** Copyright (c) by GC *****************************
static void turnleft()
// * FUNCTION		: Turning left with the motors
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	//lewy 0, prawy 1
	pwm_setup(R_PWM,"run","1");
	pwm_setup(L_PWM,"run", "0");
}

//PH****************** Copyright (c) by GC *****************************
static void go()
// * FUNCTION		: Both motors on
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	GPIOChange(1010);

	pwm_setup(R_PWM,"run","1");
	pwm_setup(L_PWM,"run", "1");
}
//PH****************** Copyright (c) by GC *****************************
static void stop()
// * FUNCTION		: Both motors off
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	pwm_setup(R_PWM,"run","0");
	pwm_setup(L_PWM,"run", "0");

	pwm_setup(L_PWM,"duty",L_SPEED);
	pwm_setup(R_PWM,"duty",R_SPEED);
	speed = atoi(L_SPEED);

	GPIOChange(1010);
	move = 0;
}
//PH****************** Copyright (c) by GC *****************************
static void turn_around()
// * FUNCTION		: Both motors on
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	turnleft();
	usleep(3);
}

static void backward() // go backward
{
	GPIOChange(101);
	pwm_setup(R_PWM,"run","1");
	pwm_setup(L_PWM,"run", "1");
}

static void right_forward(int dir) // turn right while going forw/backw
{
	if(dir == 2) GPIOChange(1010);
	else if(dir == 1) GPIOChange(101);

	pwm_setup(L_PWM,"duty","5500");
	pwm_setup(R_PWM,"duty","8800");
}

static void left_forward(int dir)
{
	if(dir == 2) GPIOChange(1010);
	else if(dir == 1) GPIOChange(101);

	pwm_setup(R_PWM,"duty","5500");
	pwm_setup(L_PWM,"duty","8800");
}

static void GPIOChange(int gpio) // L1, L2, R1, R2
{
	if(gpio % 10 == 1)
		gpio_setvalue(R_GPIO_2,"1");
	else
		gpio_setvalue(R_GPIO_2,"0");

	if((gpio/10) % 10 == 1)
		gpio_setvalue(R_GPIO_1,"1");
	else
		gpio_setvalue(R_GPIO_1,"0");

	if((gpio/100) % 10 == 1)
		gpio_setvalue(L_GPIO_2,"1");
	else
		gpio_setvalue(L_GPIO_2,"0");

	if((gpio/1000) % 10 == 1)
		gpio_setvalue(L_GPIO_1,"1");
	else
		gpio_setvalue(L_GPIO_1,"0");
}
