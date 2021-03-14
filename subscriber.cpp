#include<iostream> 
#include<string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "helpers.h"

    
using namespace std;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret; // creeaza socket
	struct sockaddr_in serv_addr; // creeaza o adresa
	string message; // bufferul in care se stocheaza date ?
    char buffer[BUFLEN];
	string id_client;
	int flag = 1;

	if (argc < 4) { // suficiente argumete
		usage(argv[0]);
	}

	DIE(strlen(argv[1]) > 10, "ID must have maximum 10 characters.\n");
	

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar

	


	// socket tcp
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	// crearea adresei
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	// conectarea unui socket
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "Error connecting to server.\n");

	// trimitere id
	n = send(sockfd, argv[1], strlen(argv[1]) + 1, 0);
	DIE(n < 0, "Couldn't send client id.\n");
	if (n == 0) {
		cout << "nu stiu sigur\n";
	}

	//noul file descriptor
	FD_ZERO(&read_fds); // 
	FD_ZERO(&tmp_fds);
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	// fdmax = sockfd;

	// trimite id-ul serverului
	

	// bool send_id = true;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

	while (1) {
		tmp_fds = read_fds;

		ret = select (sockfd + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin); // citeste de la tastatura de aici;

			// daca da exit se inchide
			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}
            
			// verificare comanda
            vector<string> message = fromCharToSubscribeMessage(buffer);

			// daca primul element are valoarea 1 atunci comanda nu este buna
            if (message[0].size() == 1){
				// se afiseaza mesajul corespunzator pentru acea eroare
                cout << message[1] << "\n";
            } else {
                // comanda este buna si poate fi trimisa catre serve
                n = send(sockfd, buffer, sizeof(buffer), 0);
				// in caz de eroare
			    DIE(n < 0, "Send error.\n");
				// daca mesajul de (un)subscribe a fost trimis afiseaza acest mesaj
				if (n >= 0) {
                	cout << message[0] << "d " << message[1] << "\n";
				}
            }

			
		}

		if (FD_ISSET(sockfd, &tmp_fds)) {
			// daca a primit mesaj de la server acesta va fi afisat
			// mesajul va veni deja in format human-readable
			memset(buffer, 0, 1600);
			n = recv(sockfd, buffer, sizeof(buffer), 0);
			if (n == 0) {
				// cout << "Serverul a inchis conexiunea.\n";
				break;
			}
			DIE(n < 0, "Didn't receive message from server.\n");
       		printf("%s\n", buffer);
			
		}

	}

	close(sockfd);

	return 0;
}