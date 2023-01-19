#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include "fichier.h"
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>



int main(int argc, char ** argv){
	
	if(argc!=4){
		printf("Wrong usage. \n Please use %s hostname port file", argv[0]);
		exit(EXIT_FAILURE);
				}
	
	printf("Welcome to 2G2TD2's implementation of a tftp client \n");
	printf("Sending %s to the host : %s@%s\n", argv[3], argv[1], argv[2]);


	
	/* Mise en place du socket pour la communication entre l'utilisateur et le serveur en utilisant le protocole UDP */
	
	struct addrinfo hints;
	struct addrinfo *res;
	memset(&hints,0,sizeof(hints));
	
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_UDP;
	
	getaddrinfo(argv[1],argv[2],&hints,&res);
	struct addrinfo * copie =res;
	
	char buffer[128]= {0};
	char server[128]= {0};
	while(copie!=NULL){
		
		printf("\t ai_protocol =%d :\n", copie->ai_protocol);
		getnameinfo(copie->ai_addr, copie->ai_addrlen, buffer, 128, server, 128, NI_NUMERICHOST | NI_NUMERICSERV );
		printf("\t adresse IP : %s\n", buffer);
		copie = copie->ai_next;
		}

		
	
	/* Ecriture de la requete que l'on envoie au serveur (RRQ Packet) détaillé dans le RFC 1350*/
	
	
	char requete_lecture[MAXSIZE] = {0};
	requete_lecture[1] = 1;
	sprintf(requete_lecture + 2, "%s", argv[3]);
	sprintf(requete_lecture + 3 + strlen(argv[3]), "NETASCII");
	
	char retour[MAXSIZE] = {0};
	
	
	int sock = socket(res->ai_family, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == -1){
			perror("sock");
			exit(EXIT_FAILURE);
			}
	
	/* On envoie la requete au serveur puis on attend sa réponse */
	
	sendto(sock, requete_lecture, strlen(argv[3])+12, 0, res->ai_addr, res->ai_addrlen);
	
	
	struct sockaddr from;
	int addrlen;
	int Yo=1;
	while(Yo==1){
		int n=recvfrom(sock, retour, 516, 0, &from, &addrlen);
		
		/* Nous recevons les trames de data provenant du serveur. On séléctionne alors les n octets de datas que l'on va écrire dans le fichier ./fichier.txt */
		
		int BlockNumber = retour[3];
		char acknowledge[4]={0,4,0,0};
		char Fichier[MAXSIZE]={0};
		int fd = open("./fichier.txt", O_CREAT|O_WRONLY, S_IRWXO|S_IRWXU|S_IRWXG);
	
	
		if (retour[1]==5){
			printf("I got an error message : \n%s\n",retour+4);
			}
		
		if (retour[1]==3){
			for (int i=0;i<n-4;i++){
				Fichier[i+512*(BlockNumber-1)] = retour[i+5];
			}
			//Fichier[512*(BlockNumber)]='\0';
			write(fd,Fichier,n-4);
			close(fd);
			acknowledge[2]=retour[2];
			acknowledge[3]=retour[3];
			sendto(sock, acknowledge, 4, 0, &from, addrlen);		/* On envoie un message au serveur pour lui dire que l'on a bien recu la trame*/
			if (n!=516){											/* On regarde si la trame est complete ou non, c'est à dire si il reste des trames a venir ou si celle-ci etait la derniere*/
			Yo=0;													/* Si la trames est complete, d'autres vont venir, alors on réitére le procédé juqu'a l'obtention d'une trame incomplete */
			}
		}
	}	
	return 0;
	
	close(sock); 
}

	
