/*
 * Copyright (C) 2018 Bill Bao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BUFFER_LENGTH 256               // The max buffer length
static char receive[BUFFER_LENGTH];     // The receive buffer from the mockware device

#define DEVICE_NAME "/proc/mockware"

int main(){
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   
   fd = open(DEVICE_NAME, O_RDWR);             // Open the device with read/write access
   if (fd < 0) {
      perror("Failed to open the device...");
      return errno;
   }
   printf("Type in a short string to send to the mockware device:\n");
   scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)
   printf("Writing message to the device [%s].\n", stringToSend);
   ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the mockware device
   if (ret < 0) {
      perror("Failed to write the message to the device.");
      return errno;
   }
 
   printf("Press ENTER to read back from the device...\n");
   getchar();
 
   printf("Reading from the device...\n");
   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the mockware device
   if (ret < 0) {
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: [%s] readed=%d\n", receive, ret);

   return 0;
}