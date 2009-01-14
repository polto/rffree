/*!***************************************************************************
 *! FILE NAME  : RFfree.c
*! DESCRIPTION: read values from RFid antena connected to Elphel 353 camera.
*! Copyright (C) 1999-2009 Alsenet SA
*! -----------------------------------------------------------------------------**
*!  This program is free software: you can redistribute it and/or modify
*!  it under the terms of the GNU General Public License as published by
*!  the Free Software Foundation, either version 3 of the License, or
*!  (at your option) any later version.
*!
*!  This program is distributed in the hope that it will be useful,
*!  but WITHOUT ANY WARRANTY; without even the implied warranty of
*!  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*!  GNU General Public License for more details.
*!
*!  You should have received a copy of the GNU General Public License
*!  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*! -----------------------------------------------------------------------------**
*/

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

#define BAUDRATE B38400
#define DEVICE "/dev/ttyUSB1"
#define BUFF_SZ 255
#define FALSE 0
#define TRUE 1

void signal_alarm (int status);
void signal_io (int status);
void readTTY();
void send_cmd(char *text, char *cmd, int cmd_sz);

int wait_flag;
int fd;

void main() {
   struct termios oldtio, newtio;       //place for old and new port settings for serial port
   struct sigaction saio;               //definition of signal action
   int n;
   
   fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
   if (fd < 0) {
    perror(DEVICE);
    exit(-1);
   }

   saio.sa_handler = signal_alarm;
   sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
   saio.sa_flags = 0;
   saio.sa_restorer = NULL;
   sigaction(SIGALRM,&saio,NULL);

   //install the serial handler before making the device asynchronous
   saio.sa_handler = signal_io;
   sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
   saio.sa_flags = 0;
   saio.sa_restorer = NULL;
   sigaction(SIGIO,&saio,NULL);

   // allow the process to receive SIGIO
   fcntl(fd, F_SETOWN, getpid());
   // Make the file descriptor asynchronous (the manual page says only
   // O_APPEND and O_NONBLOCK, will work with F_SETFL...)
   fcntl(fd, F_SETFL, FASYNC);

   tcgetattr(fd,&oldtio); // save current port settings 
   // set new port settings for canonical input processing 
   newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0;
   newtio.c_lflag = 0;       //ICANON;
   newtio.c_cc[VMIN]=1;
   newtio.c_cc[VTIME]=0;
   tcflush(fd, TCIFLUSH);
   tcsetattr(fd,TCSANOW,&newtio);

   send_cmd("Sending ON", "\200\150\000\234", 4);
   send_cmd("Sending SIGNAL", "\200\146\002\003\012\377", 6);
//   n=16;
//   while ((n--)>0) {
    send_cmd("Sending ANTICOLLISION", "\200\064\002\002\000\232", 6);
//   }      
   send_cmd("Sending OFF", "\200\152\000\015", 4);

   tcsetattr(fd,TCSANOW,&oldtio);
   close(fd);
}

void send_cmd(char *text, char *cmd, int cmd_sz) {
 printf("%s\n", text);
 write(fd, cmd, cmd_sz);
 alarm(1);
 wait_flag = TRUE;
 readTTY();
}

void readTTY() {
 char buf[BUFF_SZ];
 int i, res;
 
 while (wait_flag) {
  res = read(fd, buf, BUFF_SZ);
  if (res) {
   printf(" ");
   for (i=0; i<res; i++) 
    printf("%02hhX ", buf[i]);
   printf("\n");
  }
 }
}

void signal_alarm (int status) {
   wait_flag = FALSE;
}
void signal_io (int status) {
//   wait_flag = FALSE;
}
