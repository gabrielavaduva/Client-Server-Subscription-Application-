#ifndef _SUBS_H
#define _SUBS_H 1

#include<iostream> 
#include<string>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <math.h>
using namespace std;
// #define BUFLEN 1600
class Message_from_udp{
 public:
    char ip[20];
    int port;
    string topic;
    unsigned char type;
    char value[1501];
    Message_from_udp(){}

    void set_type_and_value(char *buffer) {
        long long number;
        double number_d;
        // seteaza type
        type = buffer[50];
        char data[1502];


        int i = 51;
        char c = buffer[51];
        
        int index = 0;

        // mesajul in format char[]
        while((i < 1551) && (c != '\0')) {
            data[index] = buffer[i];
            i++;
            c = buffer[i];
            index++;
        }

        memcpy(data, buffer + 51, strlen(buffer) + 1);
        if (type == 0) {
            number = ntohl(*(uint32_t*)(data + 1));
            if (data[0] == 1) {
                number = -number;
            }
            string num = to_string(number);
            int j = 0;
            while (num[j] != '\0'){
                value[j] = num[j];
                j++;
            }
            value[j] = '\0';

        } else if (type == 1) {
            number_d = ntohs(*(uint16_t*)(data));
            number_d = number_d / 100;
            string num = to_string(number_d);
            int j = 0;
            while (num[j] != '\0'){
                value[j] = num[j];
                j++;
            }
            value[j] = '\0';

        } else if (type == 2) {
            number_d = ntohl(*(uint32_t*)(data + 1));
            
            number_d = number_d / pow(10, data[5]);

            if (data[0] == 1) {
                number_d = -number_d;
            }

            string num = to_string(number_d);
            int j = 0;
            while (num[j] != '\0'){
                value[j] = num[j];
                j++;
            }
            value[j] = '\0';

        } else if (type == 3) {
            memcpy(value, data, strlen(data) + 1);
            value[strlen(data) + 2] = '\0';
        }
    }
};

class Subscriber{
 public:
    string id;
    int socket;
    vector<pair<string, int>> topics_stored;
    vector<Message_from_udp> messages; 
    bool online;


    Subscriber(string id, int socket) {
        this->id = id;
        this->socket = socket;
        online = true;
    }

    void add_topic(string topic, int sf) {
        topics_stored.push_back(make_pair(topic, sf));
    }

    void add_message(Message_from_udp mesaj) {
        messages.push_back(mesaj);
    }

    bool isOnline() {
        return online;
    }

    void deconnectSubscriber() {
        online = false;
    }
};



class Topic {
 public:
    string topic;
    unordered_map<string, bool> subscribers;

    Topic(string topic) {
        this->topic = topic;
    }

    void subscribe_client(string id_sub, bool sf) {
        subscribers.insert(make_pair(id_sub, sf));
    }

    void unsubscribe_client(string sub) {
        subscribers.erase(sub);
    }

};


#endif
