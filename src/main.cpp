/* 	File: 		main.c
* 	Author: 	Progga Deb
*	
*	Base code provided by Michael Thoreau: https://github.com/michaelthoreau/stm32-action-recognition 
* 	LIS3DSH library provided by Grant Phillips:	https://os.mbed.com/users/grantphillips/code/LIS3DSH/ 
*/

/* system imports */
#include <mbed.h>
#include <math.h>
#include <USBSerial.h>

/* user imports */
#include "LIS3DSH.h"


/* USBSerial library for serial terminal */
USBSerial serial(0x1f00,0x2012,0x0001,false);

/* LIS3DSH Library for accelerometer  - using SPI*/
LIS3DSH acc(PA_7, SPI_MISO, SPI_SCK, PE_3);

/* Timer Object */
Timer t;

/* LED output */
DigitalOut ledSit(LED3);
DigitalOut ledPush(LED4);
DigitalOut ledJJ(LED5);
DigitalOut ledSquat(LED6);
DigitalOut ledCount(LED4);
DigitalOut ledCompleted(LED5);

/* Button input */
DigitalIn button(BUTTON1);

/* Rep count definition */
#define MAX_REPS 5;


/* State type definition */
typedef enum state_t {
	lying,
	sitting,
	standing,
	push_up_down,
	push_up,
	squat_down,
	jump, 
	reset
} state_t;

/* Blink to get ready for next exercise */
void get_ready() {
	int j = 10;
	while( j > 0) {
		ledCompleted = 1;
		wait_ms(600);
		ledCompleted = 0;
		wait_ms(400);

		j--;
	}
}


/* Blink ledCount for # of reps remaining */ 
void display_count(int counts, DigitalOut ledOut) {
	ledOut = 1; 	// corresponding exercise led output
	int i = counts + 1;
	
	if (i <= 0)			// check if reps are complete
	{
		ledCompleted = 1;
		wait_ms(1000);
		ledCompleted = 0;
	}
	else {
		while(i > 0) {			// blink # of reps left
			ledCount = 1;
			wait_ms(500);
			ledCount = 0;
			wait_ms(500);
			i = i - 1;
		}
	}

	ledOut = 0;
	wait_ms(2000);		// wait 2 secs before repeating (if button is held down)
}

/* Filter average of data samples */
float filter_avg(float *ringbuf, uint8_t N) {
	float avg = 0;

	/* add all samples */
	for (uint8_t i = 0; i < N; i++) {
		avg += ringbuf[i];
	}

	/* divide by number of samples */
	avg /= (float)N;

	return(avg);
}


int main() {
	int16_t X, Y;							// x and y coordinates
	int16_t zAccel = 0; 					// acceleration in z (raw)
	float g_z = 0;  						// acceleration in z (g force)
    float angle = 0;						// angle relative to z axis
	const float PI = 3.1415926;				// pi

	state_t state = reset;					// initial state set to sit-up starting position (lying down)

	/* initial rep counter definition */
	int counts_Sit = MAX_REPS;
	int counts_Push = MAX_REPS;
	int counts_Squat = MAX_REPS;
	int counts_JJ = MAX_REPS; 

	/**** Filter Parameters  ****/
    const uint8_t N = 20; 					// filter length
    float ringbuf[N];						// sample buffer
    uint8_t ringbuf_index = 0;				// index to insert sample


	/* check detection of the accelerometer */
	while(acc.Detect() != 1) {
        printf("Could not detect Accelerometer\n\r");
       	wait_ms(200);
    }

	while(1) {

		/* read data from the accelerometer */
		acc.ReadData(&X, &Y, &zAccel);

		/* normalise to 1g */
		g_z = (float)zAccel/18263.0;

		/* insert in to circular buffer */
		ringbuf[ringbuf_index++] = g_z;

		/* at the end of the buffer, wrap around to the beginning */
		if (ringbuf_index >= N) {
			ringbuf_index = 0;
		}


		/********** START of filtering ********************/

		/* get mean of all data samples */
		float g_z_filt = filter_avg(ringbuf, N);

		/* restrict to 1g (acceleration not decoupled from orientation) */
		if (g_z_filt > 1) {
			g_z_filt = 1;
		}

		/********** END of filtering **********************/
		


		/* compute angle in degrees */
		angle = 180*acos(g_z_filt)/PI;


		/* State machine transitions */
		switch (state) {
			case lying: // lying down (start: sit-ups)
				ledSit = 1;		// exercise in progress 

				if(button) {
					display_count(counts_Sit, ledSit);	// blink remaining reps
				}

				if ((angle > 45) && (counts_Sit >= 0)) {	// transition to sitting state
						state = sitting;				
				}
				else {	// exercise completed	
					ledSit = 0;			
					state = reset;	// transition to reset state					
				}
				break;



			case sitting:	// sitting up (end: sit-ups)
				 ledSit = 1;	// exercise in progress 

				if(button) {
					display_count(counts_Sit, ledSit);	// blink remaining reps
				}

				if ((angle < 30) && (counts_Sit >= 0)) {
					counts_Sit--;		// decrement exercise count 
					state = lying;		// transition to lying state
				}
				else{	// exercise completed
					ledSit = 0;
					state = reset;	// transition to reset state
				}
				break;



			case standing:	// standing vertical (start: jumping jacks, squat)
				
				if(button && (ledSquat == 1)) {
					display_count(counts_Squat, ledSquat);	// blink remaining reps for squats
				}
				else if(button && (ledJJ == 1)) {
					display_count(counts_JJ, ledJJ);	// blink remaining reps for jumping jacks
				}

				if ((angle < 65) && (angle > 20) && (counts_Squat >= 0)){	
					state = squat_down;		// transition to squat_down state
				}
				else if(counts_JJ >= 0) {	
					state = jump;			// transition to jump state
				}
				else {		// exercise completed
					ledSquat = 0;	
					ledJJ = 0;		
					state = reset;	// transition to reset state
				}
				break;



			case push_up_down:	// pushing down (end: push-up)
				ledPush = 1;

				if(button) {
					display_count(counts_Push, ledPush);	// blink remaining reps
				}

				if ((angle < 110) && (angle <= 145) && (counts_Push >= 0)) {
					counts_Push--;		// decrement exercise count
					state = push_up;	// transition to pushing up state
				}
				else{	// exercise completed
					ledPush = 0;	
					state = reset;	// transition to reset state
				}
				break;



			case push_up:	// pushing up (start: push-up)
				ledPush = 1;

				if ((angle >= 100) && (angle <= 125)) {
					state = push_up_down;		// transition to pushing down state
				}
				
				else {		// exercise completed
					ledPush = 0;	
					state = reset;	// transition to reset state
				}
				break;



			case squat_down:	// squatting down (end: squat)
				ledSquat = 1;

				if ((angle > 85) && (counts_Squat >= 0)) {
					counts_Squat--;	// decrement exercise count
					state = standing;	// transition to standing state
				}
				else {		// exercise completed
					ledSquat = 0;
					state = reset;	// transition to reset state
				}
				break;
			


			case jump:	// jumping up (end: jumping jacks)
				ledJJ = 1;

				if ((angle > 85) && (counts_JJ >= 0)) {
					counts_JJ--;	// decrement exercise count
					state = standing;	// transition to standing state
				}
				else {	// exercise completed
					ledJJ = 0;
					state = reset;	// transition to reset state
				}
				break;



			case reset:	// detect exercise using starting position data
				get_ready();	// blink red LED for 10 seconds to get into starting position 
				
				/* Detect which exercise user is getting ready to do next*/
				if ((angle <= 30)) {
					state = lying;	// starting position for sit-up
				}
				if ((angle >= 60) && (angle <= 80)) {
					state = push_up;		// starting position for push-up
				}
				if ((angle >= 85)) {
					state = standing;		// starting position for squat/jumping jacks
				}
				break;



			default:
				printf("If this prints, something went wrong.\n\r");
		}

		/* print angle - avoid doing this in a time sensitive loop */
		// serial.printf("angle: %.2f degrees \r\n", angle);
		// serial.printf("%d, %d, %d\n\r", X, Y, zAccel);
		// serial.printf("x: %d, y: %d, z: %d, angle: %.2f\n\r", X, Y, zAccel, angle);

		
		/* sample rate is very approximately 10Hz  - better would be to use the accelerometer to trigger an interrupt*/		
		wait_ms(100);
	}
	
}



