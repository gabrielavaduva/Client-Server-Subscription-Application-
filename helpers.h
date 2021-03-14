#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <iostream> 
#include <string>
#include <vector>
#include "subs.h"
#include <stdlib.h>

using namespace std;

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN		256	// dimensiunea maxima a calupului de date
#define MAX_CLIENTS	5	// numarul maxim de clienti in asteptare
#define MAX_ID 10
#define TOPIC_LEN 50

string get_topic(char *buffer){
    string topic;
    char c = buffer[0];
    int i = 0;
    while(c != '\0'){
        topic = topic + c;
        i++;
        c = buffer[i];
    }
    return topic;
}

void make_buffer_from_message(char buf[], Message_from_udp msg){
    string port = to_string(msg.port); // port
    string topic = msg.topic; //topic
    char value[1500]; // value
    memcpy(value, msg.value, strlen(msg.value) + 1);
    char ip[20];
    memcpy(ip, msg.ip, strlen(ip) + 1);
    string type;
    if (msg.type == 0) {
        type = "INT";
    } else if (msg.type == 1) {
        type = "SHORT_REAL";
    } else if (msg.type == 2) {
        type = "FLOAT";
    } else if (msg.type == 3) {
        type = "STRING";
    } else {
        DIE(1, "Wrong type.\n");
    }

    string first_part = ":" + port + " client_UDP - " + topic + " - " + type + " - ";

    int index = 0;
    while (ip[index] != '\0') {
        buf[index] = ip[index];
        index++;
    }

    int index_str = 0;
    while(first_part[index_str] != '\0') {
        buf[index] = first_part[index_str];
        index++;
        index_str++;
    }

    int index_val = 0;
    while(value[index_val] != '\0') {
        buf[index] = value[index_val];
        index++;
        index_val++;
    }
    buf[index] = '\0';

}




string fromCharArrayToString (char *buffer, int max_len) {
    char c = buffer[0];
    int index = 0;
    string id;
    while ((c != '\0') && (c != '\n') && (index < max_len)) {
        id = id + c;
        index++;
        c = buffer[index];
    }

    return id;
}

vector<string> fromCharToSubscribeMessage(char buffer[]) {
    vector<string> result;
    char c = buffer[0];
	int pos = 0;
	int noWords = 0;
	string sub;
	string topic;
    string SF;
    while (c != '\0') {
		// noWords++;
		if (noWords > 2) {
            result.push_back("0");
            result.push_back("Error : too many words");
			return result;
		}
        while ((c != ' ') && (c != '\0') && (c != '\n')) {
			if (noWords == 0) {
				sub = sub + c;
			} else if (noWords == 1) {
				topic = topic + c;
			} else if (noWords == 2) {
                SF = SF + c;
            }
			pos++;
			c = buffer[pos];
		}
        
        pos++;
		c = buffer[pos];
        if (c == ' ') {
            continue;
        } else {
            noWords++;
        }
    }
    if (noWords < 2) {
        result.push_back("1");
        result.push_back("Error: few words");
		return result;
    }

    string subs ("subscribe");
    string unsubs("unsubscribe");

    if ((sub.compare(unsubs) != 0) && (sub.compare(subs) != 0)) {
            result.push_back("2");
            result.push_back("Invalid command.");
            return result;
    }

    if (sub[0] == 's') {
        if ((SF.compare("1") != 0) && (SF.compare("0") != 0)) {
                result.push_back("3");
                result.push_back("Wrong sf.");
                return result;
        }
    }

    result.push_back(sub);
    result.push_back(topic);
    result.push_back(SF);
    return result;
}


void unsubcribe_from_all(unordered_map<string, Topic> topics, string id_client) {
    unordered_map<string, Topic>::iterator it;
    for (it = topics.begin(); it != topics.end(); it++) {
        (*it).second.unsubscribe_client(id_client);
    }
}

#endif
