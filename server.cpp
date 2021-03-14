#include<iostream> 
#include<string>
#include <cstring>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "subs.h"

#define FD_START 4
#define BUF_MAX 1600
// #define MAX_CLIENTS 100
using namespace std;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}
// aceasta functie trimite mesajele tuturor in afara de el insusi
// void forward_ex1(int *clients, int client_src, char *buffer) {

// 	for (int i = 0; i < MAX_CLIENTS; i++) {
// 		if (clients[i] && i != client_src) {
// 			send(i + FD_START, buffer, strlen(buffer), 0);
// 			printf("Sent to %d\n", i + FD_START);
// 			break;
// 		}
// 	}
// }


int main(int argc, char *argv[]) {
    int sockfd_tcp, sockfd_udp;
    int newsockfd, portno;
	int flag = 1;
	string message;
	char buffer[BUF_MAX];
    struct sockaddr_in serv_addr, sub_addr, cli_addr;
	socklen_t clilen;

	unordered_map<string, Subscriber> clients;
	unordered_map<string, Topic> topics;
	unordered_map<int, string> client_sockets;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	// creare socket tcp
	sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd_tcp < 0, "Socket tcp error.\n");

	// creare socket udp
	sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfd_udp < 0, "Socket udp error.\n");

	// portno va primi numarul portului
	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	int n, i, ret;
	// vom initializa adresa sockaddr_in a tcp
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	sub_addr.sin_family = AF_INET; //IPv4
	sub_addr.sin_port = htons(portno); // portul
	sub_addr.sin_addr.s_addr = INADDR_ANY; // adresa

	// partea 1 - clienti tcp

	ret = bind(sockfd_tcp, (struct sockaddr *) &sub_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "Bind tcp client error.\n");

	// partea 2 - clienti udp
	// initializare client udp

	memset((char *) &cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET; //IPv4
	cli_addr.sin_port = htons(portno); // portul
	cli_addr.sin_addr.s_addr = INADDR_ANY; // adresa

	ret = bind(sockfd_udp, (struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "Bind udp client error.\n");

	// listen pe socket-ul tcp
	ret = listen(sockfd_tcp, MAX_CLIENTS);
	DIE(ret < 0, "Listen error");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd_tcp, &read_fds);
	FD_SET(sockfd_udp, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = sockfd_tcp;

	bool not_exit = true;

	while(not_exit) {
		tmp_fds = read_fds;

		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "Select error.\n");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == 0) {
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					if (strncmp(buffer, "exit", 4) == 0) { 
						not_exit = false;
						break;
					} else {
						printf("Invalid command.\n");
					}
				}
				if (i == sockfd_udp) {
					int bytes_read;
					uint32_t len;
					
					memset(buffer, 0, BUF_MAX);
					bytes_read = recvfrom(sockfd_udp, buffer, BUFLEN,
						 0, (struct sockaddr *) &cli_addr, &len);
					DIE(bytes_read < 0, "No message from UDP client.\n");

					Message_from_udp mesaj;
					char message_buffer[1600];
					
					strcpy(mesaj.ip, inet_ntoa(cli_addr.sin_addr)); // ip = construieste mesajul
					mesaj.port = ntohs(cli_addr.sin_port); // port
					
					string topic_payload = get_topic(buffer); //topic
					mesaj.topic = topic_payload; // topic
					mesaj.set_type_and_value(buffer); // type + value

					// pregateste mesajul de trimis
					
					memset(message_buffer, 0, 1600);

					make_buffer_from_message(message_buffer, mesaj); // TODO make mesage
				
					// printf("tot mesajul este %s\n", message_buffer);

					unordered_map<string, Topic> :: iterator it = topics.find(topic_payload);
					if (it == topics.end()) {
						// cout << "Didn't subscribers to this topic.\n";
						continue; // se ignora daca nu e in lista de topicuri pentru ca nu are cui sa trimita

					} else {
						unordered_map<string, bool> :: iterator sub;
						// se parcurg toti clientii la acel topic
						for (sub = (*it).second.subscribers.begin(); sub != (*it).second.subscribers.end(); sub++) {
							
							string id_client = sub->first; // id-ul de client
							bool sf = sub->second; // sf al acelui cleint
							
							int socket_client = clients.find(id_client)->second.socket; // socket-ul
							
							bool online = clients.find(id_client)->second.isOnline();
							
							if (online) {
								
								int x = send(socket_client, message_buffer, strlen(message_buffer), 0);
								if (x < 0) {
									printf("Couldn't send a message.\n");
								}
							} else if (sf) {
								// cout << id_client << " is not online.\n";
								clients.find(id_client)->second.add_message(mesaj);
							}
						}
					}

					
					

				} else if (i == sockfd_tcp) {
					
					// se conecteaza un nou client
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd_tcp, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "Error accepting tcp client.\n");

					setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}

					// clients[newsockfd - FD_START] = 1;
					// primeste id-ul clientului
					// cout << "a gasit un client nou\n";
					memset(buffer, 0, BUFLEN);
					n = recv(newsockfd, buffer, sizeof(buffer), 0);
					DIE(n < 0, "No ID received.\n");
					
					// adauga noul client impreuna cu socket-ul lui si id-ul
					
					string id_client = fromCharArrayToString(buffer, MAX_ID);

					// adauga noul client la unordered map pentru a-l retine: socket si id_client
					client_sockets.insert(make_pair(newsockfd, id_client));
					// cout <<"clientul are numarul " << newsockfd << " .\n";

					// il adauga la lista de clienti dupa id
					Subscriber subscriber(id_client, newsockfd);
					subscriber.online = true;
					
					// cauta daca acest client s-a mai conectat
					unordered_map<string, Subscriber>:: iterator sub = clients.find(subscriber.id);

					// daca nu s-a mai conectat in adauga in lista
					if (sub == clients.end()) {
						clients.insert(make_pair(id_client, subscriber));
					} else {
						// daca s-a mai conectat atunci statusul sau va fi online
						sub->second.online = true;

						// primeste un nou socket
						sub->second.socket = newsockfd;

						// primeste toate mesajele pe care le-a ratat cat fost plecat
						unordered_map<string, Subscriber>:: iterator it;
						for (Message_from_udp mess : sub->second.messages) {
							char msg_to_send[1600];
							// msg_to_send = mess.make_char_buffer();
							memset(msg_to_send, 0, 1600);
							make_buffer_from_message(msg_to_send, mess);
							// TODO send message
							send(newsockfd, msg_to_send, strlen(msg_to_send), 0);
						}
						// TODO sterge mesajele deja trimise
						sub->second.messages.clear();
					}
							
					printf("New client %s connected from %s:%d\n", buffer,
							inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));


				} else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0) {
						// conexiunea s-a inchis
						string id_client;
						if (client_sockets.find(i) == client_sockets.end()) {
							// cout << "nu s a gasit clientul " << i << " .\n";
							continue;
						} else {
							id_client = client_sockets.find(i)->second;
							clients.find(id_client)->second.online = false;
						}
						cout << "Client " << id_client << " disconnected.\n";
						// string id_client = client_sockets.find(i)->second;
						close(i);
						

						// TODO dezactivare client cu ajutorul lui map
						
						
						// TODO se seteaza online pe false
						
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
					} else {
						
						// cout << "S-a primit de la client textul: " << buffer << "\n";
						
						vector<string> subscription = fromCharToSubscribeMessage(buffer);
						string id_client;

						if (client_sockets.find(i) == client_sockets.end()) {
							// cout << "nu s a gasit clientul " << i << " .\n";
							continue;
						} else {
							id_client = client_sockets.find(i)->second;
						}
						string topic = subscription[1];
						
						if (subscription[0][0] == 's') {
							// atunci de aboneaza
							bool sf;
							// verificare sf
							if (subscription[2][0] == '1') {
								sf = true;
							} else {
								sf = false;
							}

							if (topics.find(subscription[1]) != topics.end()){
								// daca exista acel topic se adauga id-ul clientului si sf-ul
								topics.find(subscription[1])->second.subscribe_client(id_client, sf);
							} else {
								// daca nu exista se aduga topicul impreuna cu sf-ul
								Topic topic_to_add(topic);
								topic_to_add.subscribe_client(id_client, sf);
								topics.insert(make_pair(topic, topic_to_add));
							} 
						} else {
							if (topics.find(subscription[1]) != topics.end()){
								// daca exista acel topic
								topics.find(subscription[1])->second.unsubscribe_client(id_client);
							} else {
								// cout << "This topic doesn't exist -> can't unsubscribe.\n";
							} 
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < fdmax; i++) {
		close(i);
	}

	return 0;

}
