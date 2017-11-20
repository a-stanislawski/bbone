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

static void LEDChangeState(FILE *f, const char *L, int on);
static void LEDChange(FILE *f[], const char *L[], int state);


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
