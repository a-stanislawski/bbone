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

#define L_PWM 		"pwm_test_P9_42.15"
#define R_PWM 		"pwm_test_P9_22.16"
#define L_GPIO 		"/sys/class/gpio/gpio49"

#define L_GPIO_1	"27"
#define L_GPIO_2	"46"
#define R_GPIO_1	"44"
#define R_GPIO_2	"45"

#define L_SPEED		"7500"
#define R_SPEED		"7500"

static void am33xx_pwm();
static void pwm_init(char pin[]);
static void gpio_init(char pin[]);
static void gpio_uninit(char pin[]);
static void gpio_output(char pin[]);


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

