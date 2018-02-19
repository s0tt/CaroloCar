#include <stdio.h>      // standard input / output functions
#include <iostream>
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

#include "Serial.h"

const char* PORT_NAME = "/dev/ttyUSB0";

int Serial::init_serial_simple() {
	return open(PORT_NAME, O_RDWR | O_NOCTTY);
}

int Serial::init_serial() {
	
	int port = open(PORT_NAME, O_RDWR | O_NOCTTY);

	struct termios tty;
	struct termios tty_old;
	memset (&tty, 0, sizeof tty);

	/* Error Handling */
	if (tcgetattr (port, &tty) != 0) {
	   std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
	}

	/* Save old tty parameters */
	tty_old = tty;

	/* Set Baud Rate */
	cfsetospeed (&tty, (speed_t)B9600);
	cfsetispeed (&tty, (speed_t)B9600);

	/* Setting other Port Stuff */
	tty.c_cflag     &=  ~PARENB;            // Make 8n1
	tty.c_cflag     &=  ~CSTOPB;
	tty.c_cflag     &=  ~CSIZE;
	tty.c_cflag     |=  CS8;

	tty.c_cflag     &=  ~CRTSCTS;           // no flow control
	tty.c_cc[VMIN]   =  1;                  // read doesn't block
	tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);

	/* Flush Port, then applies attributes */
	tcflush(port, TCIFLUSH);
	if (tcsetattr (port, TCSANOW, &tty) != 0) {
	   std::cout << "Error " << errno << " from tcsetattr" << std::endl;
	}
	return port;
}

int Serial::write_float(int port, float f) {

	char buffer[64];
	int ret = snprintf(buffer, sizeof buffer, "%f", f);
	if(ret < 0) {
		return -1;
	}
	else if(ret > sizeof buffer) {
		return -1;
	}
	return Serial::write_serial(port, buffer);
}
int Serial::write_serial(int port, const char* text) {

	int n_written = 0;
	int i = 0;
	do {
	    n_written += write(port, &text[i], 1);
	    i++;
	} while (text[i] != NULL && n_written > 0);

	return n_written;
}

const char* Serial::read_serial(int port) {

	int n = 0,
	    spot = 0;
	char buf = '\0';

	/* Whole response*/
	char response[1024];
	memset(response, '\0', sizeof response);

	do {
	    n = read(port, &buf, 1);
	    sprintf(&response[spot], "%c", buf);
	    spot += n;
	} while(buf != '\r' && n > 0);

	if (n < 0) {
	    std::cout << "Error reading: " << strerror(errno) << std::endl;
	}
	else if (n == 0) {
	    std::cout << "Read nothing!" << std::endl;
	}
	else {
	    std::cout << "Response: " << response << std::endl;
	}
	return response;
}
