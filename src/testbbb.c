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

static void filecopy(FILE *, FILE *);
static void removeTrigger();

static void gpio_init(char pin[]);
static void gpio_uninit(char pin[]);
static void gpio_output(char pin[]);
static void gpio_input(char pin[]);
static void gpio_setvalue(char pin[], char *value);

static void am33xx_pwm();
static void pwm_init(char pin[]);
static void pwm_uninit(char number[]);
static void pwm_setup(char pin[], char set[], char *value);

static void turnleft();
static void turnright();
static void go();
static void stop();
static void turn_around();
static void backward();

static void right_forward(int dir);
static void left_forward(int dir);

static void reverse_left();
static void reverse_right();

static void circle();

static void LEDChangeState(FILE *f, const char *L, int on);
static void LEDChange(FILE *f[], const char *L[], int state);
static void GPIOChange(int gpio);

static int move = 0; // 0 - stop, 1 - going backwards, 2 - going forwards
static int speed = 7500;

int main(void) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	int n = 0;
	unsigned char buf = '\0';
	unsigned char buf2[5] = "0";
	int USB = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY /*| O_NDELAY*/);

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
	pwm_init("bone_pwm_P9_22");
	am33xx_pwm();

	usleep(500000);

	pwm_setup(L_PWM,"duty",L_SPEED);
	pwm_setup(L_PWM,"period","10000");
	pwm_setup(L_PWM,"run","0");

	pwm_setup(R_PWM,"duty",R_SPEED);
	pwm_setup(R_PWM,"period","10000");
	pwm_setup(R_PWM,"run","0");

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

			if(strncmp(response,"F",1)==0) // forward
			{
				printf("\n");
				LEDChange(LED,LEDB,1111);
				move = 2;
				go();

			}
			else if(strncmp(response,"B",1)==0) // backward
			{
				printf("\n");
				LEDChange(LED,LEDB,1001);
				move = 1;
				backward();
			}
			else if(strncmp(response,"R",1)==0) // right
			{
				printf("\n");
				LEDChange(LED,LEDB,11);

				if(move > 0)
				{
					right_forward(move);
				}
				else
				{
				//	reverse_right();
				}
			}
			else if(strncmp(response,"L",1)==0) // left
			{
				printf("\n");
				LEDChange(LED,LEDB,1100);
				if(move > 0)
				{
					left_forward(move);
				} else
				{
				//	reverse_left();
				}
			}
			else if(strncmp(response,"S",1)==0) // stop moving forw/backw
			{
				printf("\n");
				LEDChange(LED,LEDB,0);
				move = 0;
				stop();
			}
			else if(strncmp(response,"s",1)==0) // stop turning
			{
				printf("\n");

				pwm_setup(L_PWM,"duty",L_SPEED);
				pwm_setup(R_PWM,"duty",R_SPEED);
				speed = atoi(L_SPEED);
				if(move == 1)
				{
					backward();
				}
				else if(move == 2)
				{
					go();

				} else
				{
					LEDChange(LED,LEDB,0);
					move = 0;
					stop();
				}
			}
			else if(strncmp(response,"Z",1)==0) // stop & break loop
			{
				printf("\n");
				LEDChange(LED,LEDB,0);
				stop();
				break;
			}
			else if(strncmp(response,"H",1)==0) // turn around
			{
				printf("\n");
				LEDChange(LED,LEDB,0);
				turn_around();
				break;
			}
			else if(strncmp(response,"f",1)==0) // add speed
			{
				if(speed > 0) speed -= 500;
				sprintf(spd, "%d", speed);
				printf("seed: %s x00",spd);
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
//PH****************** Copyright (c) by GC *****************************
static void filecopy(FILE *ifp, FILE *ofp)
{
	int c;
	while ((c =getc(ifp)) != EOF)
	{
		putc(c, ofp);
	}
}


//PH****************** Copyright (c) by GC *****************************
static void gpio_init
(
char pin[]			//number of initialized pin
)
// * FUNCTION		: Initialization of GPIO
// *				: (e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{

	FILE *GPIOHandle = NULL;
	char *GPIOPath = "/sys/class/gpio/export";
	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "%s", pin);
	fclose(GPIOHandle);

	printf("GPIO Init - GPIO_%s \n", pin);
	printf("GPIO Init end\n");

}
//PH****************** Copyright (c) by GC *****************************
static void gpio_uninit
(
char pin[]			//number of uninitialized pin
)
// * FUNCTION		: Uninitialization of GPIO
// *				: (e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *GPIOHandle = NULL;
	char *GPIOPath = "/sys/class/gpio/unexport";
	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "%s", pin);
	fclose(GPIOHandle);

	printf("GPIO Uninit - GPIO_%s - unexported\n", pin);
	printf("GPIO Uninit end \n");
}
//PH****************** Copyright (c) by GC *****************************
static void gpio_output
(
char pin[]				// number of GPIO pin for output set
)
// * FUNCTION		: Setting output configuration for GPIO
// *				: ( pin[] - e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: GPIO pin must be initialized first
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *GPIOHandle = NULL;
	char GPIOPath[50] = "/sys/class/gpio";
	sprintf(GPIOPath, "%s/gpio%s/direction",GPIOPath, pin);

	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "out");
	fclose(GPIOHandle);

	printf("GPIO Out - GPIO_%s \n", pin);

}
//PH****************** Copyright (c) by GC *****************************
static void gpio_input
(
char pin[]				// number of GPIO pin for input set
)
// * FUNCTION		: Setting input configuration for GPIO
// *				: ( pin[] - e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: GPIO pin must be initialized first
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *GPIOHandle = NULL;
	char GPIOPath[50] = "/sys/class/gpio";
	sprintf(GPIOPath, "%s/gpio%s/direction",GPIOPath, pin);

	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "in");
	fclose(GPIOHandle);

	printf("GPIO In - GPIO_%s \n", pin);

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
static void am33xx_pwm()
// * FUNCTION		: Initialization of am33xx_pwm
// *				:
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	//pin - f.e. bone_pwm_P9_22
	//pin bone_pwm_P9_42
	FILE *PWMHandle = NULL;
	char *SLOTS = "/sys/devices/bone_capemgr.9/slots";

	PWMHandle = fopen(SLOTS, "w");
	fprintf(PWMHandle, "am33xx_pwm");
	fclose(PWMHandle);

	printf("am33xx_pwm Init end\n");
}
//PH****************** Copyright (c) by GC *****************************
static void pwm_init
(
char pin[]			// pwm path f.e. bone_pwm_P9_22
)
// * FUNCTION		: Initialization of pwm
// *				:
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *PWMHandle = NULL;
	char *SLOTS = "/sys/devices/bone_capemgr.9/slots";

	PWMHandle = fopen(SLOTS, "w");
	fprintf(PWMHandle, "%s", pin);
	fclose(PWMHandle);

	printf("PWM Init - %s \n", pin);
	printf("PWM Init end\n");
}
//PH****************** Copyright (c) by GC *****************************
static void pwm_uninit
(
char number[]		// cat $SLOTS - check the number in system
)
// * FUNCTION		: Uninitialization of pwm
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *PWMHandle = NULL;
	char *SLOTS = "/sys/devices/bone_capemgr.9/slots";

	PWMHandle = fopen(SLOTS, "w");
	fprintf(PWMHandle, "-%s", number);
	fclose(PWMHandle);

	printf("PWM Uninit - %s \n", number);
	printf("PWM Uninit end\n");
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

	//reverse_left();
}
//PH****************** Copyright (c) by GC *****************************
static void turnright()
// * FUNCTION		: Turning right with the motors
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	//lewy 1, prawy 0
	pwm_setup(R_PWM,"run","0");
	pwm_setup(L_PWM,"run", "1");
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
	//stop;
}

static void backward() // go backward
{

	GPIOChange(101);

	pwm_setup(R_PWM,"run","1");
	pwm_setup(L_PWM,"run", "1");
}

static void reverse_left() // turn left without moving
{
	GPIOChange(110);

	pwm_setup(L_PWM,"duty","6100");
	pwm_setup(R_PWM,"duty","6100");


	pwm_setup(R_PWM,"run","1");
	pwm_setup(L_PWM,"run", "1");
}



static void reverse_right() // turn right without moving
{
	gpio_setvalue(L_GPIO_1,"1"); // left
	gpio_setvalue(L_GPIO_2,"0");
	gpio_setvalue(R_GPIO_1,"0"); // right
	gpio_setvalue(R_GPIO_2,"1");

	pwm_setup(L_PWM,"duty","6100");
	pwm_setup(R_PWM,"duty","6100");

	pwm_setup(R_PWM,"run","1");
	pwm_setup(L_PWM,"run", "1");
}

static void right_forward(int dir) // turn right while going forw/backw
{
	if(dir == 2)
	{
		GPIOChange(1010);
	} else if(dir == 1)
	{
		GPIOChange(101);
	}

	pwm_setup(L_PWM,"duty","5500");
	pwm_setup(R_PWM,"duty","8800");

	//pwm_setup(R_PWM,"run","1");
	//pwm_setup(L_PWM,"run", "1");
}

static void left_forward(int dir)
{
	if(dir == 2)
	{
		GPIOChange(1010);
	} else if(dir == 1)
	{
		GPIOChange(101);
	}

	pwm_setup(R_PWM,"duty","5500");
	pwm_setup(L_PWM,"duty","8800");

	//pwm_setup(R_PWM,"run","1");
	//pwm_setup(L_PWM,"run", "1");
}



//PH****************** Copyright (c) by GC *****************************
static void circle()
// * FUNCTION		: Circle for test motors
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	go();
	sleep(1);
	turnleft();
	sleep(1);
	go();
	sleep(1);
	turnleft();
	sleep(1);
	go();
	sleep(1);
	turnleft();
	sleep(1);
	go();
	sleep(1);
	turnleft();
	sleep(1);
	stop();
}

void LEDChangeState(FILE *f, const char *L, int on)
{
	if(on){
		if((f = fopen(L, "r+")) != NULL){
			fwrite("1", sizeof(char), 1, f);
			fclose(f);
	 	}
	} else
	{
		if((f = fopen(L, "r+")) != NULL){
			fwrite("0", sizeof(char), 1, f);
			fclose(f);
	 	}
	}
}

void LEDChange(FILE *f[], const char *L[], int state) // L1, L2, R2, R1
{
	LEDChangeState(f[0], L[0], state % 10);
	LEDChangeState(f[1], L[1], (state/10) % 10);
	LEDChangeState(f[2], L[2], (state/100) % 10);
	LEDChangeState(f[3], L[3], (state/1000) % 10);
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
