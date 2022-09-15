/*
 * untitled.c
 * 
 * Copyright 2020  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define INVALID_SOCKET	-1
#define CLIENT_CONNECTED	0
#define CLIENT_DISCONNECTED	1

int etatClient;
void clientOut(int code);


int main(int argc, char **argv)
{
    
    
    //Déclarations wiringPi
    wiringPiSetup() ;
    
    //Initialisation
    int sock;
    int csock;
    struct sockaddr_in servAddr;
    struct sockaddr_in cliAddr;
    
    int triger = 4;
    int echo = 5;
    int time = 0;
    int stop = 0;
    int pwm = 1;
    double distance;
    char buffer[50];
    const void *enable;
    
    pinMode(triger, OUTPUT);
    pinMode(echo, INPUT);
    pinMode(pwm,PWM_OUTPUT);
    
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(1020);
    pwmSetClock(376);
	    

//On veut utiliser les sockets en mode connecté, protocole par défaut (TCP)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock==INVALID_SOCKET){
        perror("socket");
        exit(-1);
    }
    printf("socket [OK]\n");
    
    servAddr.sin_addr.s_addr = inet_addr("192.168.1.208"); //utilisation de n'importe quelle interface pour l'écoute
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(60006); //Port d'écoute>50000

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (int *)&enable, sizeof enable);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (int *)&enable, sizeof enable);
    
    if(bind(sock, (const struct sockaddr *)&servAddr, sizeof(servAddr))==-1){
        perror("bind");
        if(close(sock)==-1){
            perror("close");
	    }
        printf("close [OK]\n");
        exit(-1);
    }
    printf("bind [OK]\n");

    if(listen(sock, 0)==-1){
        perror("listen");
        if(close(sock)==-1){
            perror("close");
        }
        printf("close [OK]\n");
        exit(-1);
    }
    printf("listen [OK]\n");
    
    signal(SIGPIPE,&clientOut);

 
//Init bibliothèque wiringPi

  
    
    while(1) 
    {
	
        socklen_t sinsize = sizeof(cliAddr);
	
        if((csock = accept(sock, (struct sockaddr *)&cliAddr, &sinsize)) != INVALID_SOCKET)
	{
	    
            printf("New Client [OK]\n");
	
            etatClient=CLIENT_CONNECTED;
            while(etatClient==CLIENT_CONNECTED)
	    {

	    //Acquisition capteur Ultrason, avec pilotage servomoteur
							
			 
		    for(int i = 0; i < 90; i+= 20)
		    {
			
			
			// mets le triger à l'etat haut pendant 10 useconde
			digitalWrite(triger,HIGH); 
			delayMicroseconds(10);
			// met à l'etat bas 
			digitalWrite(triger,LOW);
				 
			while ( digitalRead(echo) == LOW);
			// temps à l'etat haut du capteur echo
			stop = micros();
			while ( digitalRead(echo) == HIGH);
			// temps de propagation de l'onde ultra son
			time = micros() - stop ;
		  
			// distance en cm/us
			distance = (0.0340*time)/2;
				
			printf(" la distance = %lf\n ", distance);
			       
			//convertie la distance en string dans le buffer
			sprintf(buffer,"%lf",distance);
			// envoie la distance à l'IHM
			send(csock,buffer,strlen(buffer),0);
			sleep(1);			    
			// lance le moteur avec des angles de 20 degrès
			pwmWrite(pwm,i);
			sleep(1);
		    
		    }
            }
        }   
	
        else{
            perror("accept");
            if(close(sock)==-1){
                perror("close");
            }
            printf("close [OK]\n");
            exit(-1);
	    }
    }   
    if(close(csock)==-1){
	    perror("close csock");
    }
    // ferme le socket
    printf("close csock[OK]\n");
    
    if(close(csock)==-1){
	    perror("close sock");
    }
    printf("close sock[OK]\n");

    return 0;
}

void clientOut(int code)
    {
	etatClient=CLIENT_DISCONNECTED;
    }


