// Bot Controller

// Include 

// Basic libs 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

// Joystick 
#include <sys/ioctl.h>
#include <linux/joystick.h>

// wiringPi 
#include <wiringPi.h>
#include <wiringPiI2C.h> 

// Define 
// Joystick location 0

#define JOY_DEV "/dev/input/js0"

// GPIO Pins 

#define pin22 0x22 
#define ard04 0x04

const int offPin = 7; 

int main()
{
	// Variables 
	
	int diagnosticState = 0; 
	
	int joy_fd;
	int *axis=NULL;
	
	int num_of_axis=0;
	int num_of_buttons=0
	int x, l, k;
	
	char *button=NULL, name_of_joystick[80];
	
	int *axisPrevious=NULL;
	char *buttonPrevious=NULL;
	
	unsigned char *datawrite=NULL; 
	
	struct js_event js;
	
	int i2c04; 
	
	int translator = 0; 
	
	int SpeedLeft = 0;
	int SpeedRight = 0;
	int DirectionLeft = 0;
	int DirectionRight = 0; 
	
	int offStatusCurrent = 1;
	int offStatusPrevious = 1;  
	
	// Initial Setup 
	
	wiringPiSetup(); 
	pinMode(offPin, INPUT); 
	pullUpDnControl(offPin, PUD_UP); 
	pinMode(22, OUTPUT); 
	digitalWrite(22, LOW); 
	
	i2c04 = wiringPiI2CSetup(ard04); 
			
	// Detect to see if joystick is attached 
	
	printf("Linux Joystick Scanning Program Version 1.01 \n \n");  
	printf("Connection: %d \n \n", i2c04);
	
	if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
	{
		printf( "Couldn't open joystick at location: %s \n", JOY_DEV);
		return -1;
	}
 
	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );
 
	axis = (int *) calloc( num_of_axis, sizeof( int ) );
	button = (char *) calloc( num_of_buttons, sizeof( char ) );
	
	axisPrevious = (int *) calloc( num_of_axis, sizeof( int ) );
	buttonPrevious = (char *) calloc( num_of_buttons, sizeof( char ) );
	
	datawrite = (unsigned char*) calloc (6, sizeof(unsigned char));
	
	datawrite[0] = 0; 
	datawrite[1] = 0;
	
	datawrite[2] = 0;
	datawrite[3] = 0;
	
	datawrite[4] = 0; 
	datawrite[5] = 0;
 
	printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
	, name_of_joystick
	, num_of_axis
	, num_of_buttons );
	
	fcntl( joy_fd, F_SETFL, O_NONBLOCK ); /* use non-blocking mode */
	
	printf( "Left X: Left Y: \n\n");
 
	if( num_of_axis > 3 )
	printf("Right X: Right Y: \n\n");
			
	if( num_of_axis > 5 )
	printf("DPAD X: DPAD Y: \n\n");
			
	for( x=0 ; x<num_of_buttons ; ++x )
	printf("B%d: ", x);
 
	printf("\n\n"); 

	if (diagnosticState == 1)
	{
		// Wait 1 sec
		usleep(1000000); 
		printf("# Please expand the window \n \n"); 
		printf("Press return to start scanning controller... \n");
		getchar();
	}
	
	for (l = 0; l < num_of_axis; l++)
	{
		axisPrevious[l] = 0; 
		axis[l] = 0;
	}
	
	// Infinite loop 
	
	while( 1 )  
			{
				// Copy control data to previous state 
			
				offStatusCurrent = offStatusPrevious; 
				
				offStatusCurrent = digitalRead(offPin); 
				
				// Read the joystick state 
				read(joy_fd, &js, sizeof(struct js_event));
 
				// Process the events 
				// Send updates here? 
				switch (js.type & ~JS_EVENT_INIT)
				{
					case JS_EVENT_AXIS:
					{
						axisPrevious[js.number] = axis[js.number];
						axis[js.number] = js.value;
						break;
					}
					case JS_EVENT_BUTTON:
					{
						buttonPrevious[js.number] = button[js.number]; 
						button[js.number] = js.value;
						break;
					}
				}
				
				if (diagnosticState == 1)
				{
					// Print the results  
					printf( "LSX: %6d  ", axis[0] );
					printf( "LSY: %6d  ", axis[1] );
 
					if( num_of_axis > 2 )
					printf("RSX: %6d  ", axis[2] );
 
					if( num_of_axis > 3 )
					printf("RSY: %6d  ", axis[3] );
			
					if( num_of_axis > 4 )
					printf("DPX: %6d  ", axis[4] );
			
					if( num_of_axis > 5 )
					printf("DPY: %6d  ", axis[5] );
			
					for( x=0 ; x<num_of_buttons ; ++x )
					{
						printf("B%d: %1d", x, button[x] );
						printf(":%1d ", buttonPrevious[x] ); 
					}
				
					printf("BP: %1d", offStatusCurrent);
					printf(": %1d ", offStatusPrevious); 
				}
											
				// Send output over I2C 
			
				translator = (int) (axis[1] / 128); 
				SpeedLeft = abs(translator); 
			
				if (translator < 0) DirectionLeft = 0;
				if (translator > 0) DirectionLeft = 1;
				if (translator == 0) DirectionLeft = 0;  
			
				translator = (int) (axis[3] / 128);
			
				if (translator < 0) DirectionRight = 1;
				if (translator > 0) DirectionRight = 0; 
				if (translator == 0) DirectionRight = 1; 
			
				SpeedRight = abs(translator); 

				datawrite[0] = 6;
				datawrite[1] = 1; 
					
				datawrite[2] = (unsigned char)(SpeedLeft);
				datawrite[3] = (unsigned char)(SpeedRight);
				datawrite[4] = (unsigned char)(DirectionLeft);
				datawrite[5] = (unsigned char)(DirectionRight); 
				
				write(i2c04, datawrite, 6);
				
				// Print the results of the convertion 
				
				if (diagnosticState == 1)
				{
					printf("SL: %d ", datawrite[2]);
					printf("SR: %d ", datawrite[3]); 
					printf("DL: %d ", datawrite[4]); 
					printf("DR: %d ", datawrite[5]); 
				}
			
				if (button[0] && !buttonPrevious[0])
				{

				}
			
				if (!button[0])
				{
					
				}
			
				if (button[12] && !buttonPrevious[12])
				{
					return 0; 
				}
				
				if (offStatusCurrent == 0)
				{
					system("sudo shutdown -h now");
				}
				
				if (diagnosticState == 1)
				{
					printf("  \r");
					fflush(stdout); 
				}
				
				// Delay to update 50 times a second - so it's not such a processor hog!
				// 20000us delay causes a slight read lag  
				// 10000us delay is some what better 
			
				usleep(10000); 
	}
 
close( joy_fd ); /* too bad we never get here */
return 0;
}

