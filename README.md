# Client-Server-Subscription-Application

Implementare :
- server : folosind informatiile din laboratorul 6, 7 si 8 
am realizat multiplexare. Clientii de udp si tcp au propriul 
socket.
Pentru pastrarea informatiilor referitoare la clientii tcp 
am folosit unrmatoarele structuri de date :
	unordered_map<int, string> client_sockets;
		- retinerea ID-ului clientului pe baza socket-ului
	unordered_map<string, Subscriber> clients; 
		- retinerea clientului si a informatiilor sale pe 
		baza id-ului, pentru o cautare eficienta
	unordered_map<string, Topic> topics;
		- retinerea informatiilor unui topic pe baza numelui
		sau
Am decis sa ma folosesc de id-urile clientilor deoarece acestea
nu se vor schimba si dupa deconectare.
- subscriber : acesta urmeaza intocmai modelul prezentat la laborator
impreuna cu tratarea situatiilor de eroare.

Strucuturile de date implementate de mine pentru facilitare
trimiterii mesajelor:
- Subscriber : retine id-ul, socket-ul si mesajele pe care
le primeste cat timp este offline pe care le va primi cand se
conecteaza din nou
- Topic : numele topicului si unordered_map<string, bool> 
care retine clientii abonati la acel topic si valoare sf-ului
- Message_from_udp : ip-ul clientului udp, portul, topicul, 
tipul de date si datele. Este ceea ce si subscriber-ul pastreaza
in lista sa de mesaje.

Functii implementate:
- getTopic: pentru a retine topicul unui mesaj udp si pentru a
stii la care clienti sa trimita
- make_buffer_from_message: aici tipuc Message_from_udp este
transformat in mesajul final ce va fi trimis catre un client
tcp

Alte functii care transforma datele:
- fromCharArrayToString
- fromCharToSubscribeMessage: care interpreteaza mesajul de
abonare al clientului tcp; acesta poate retine ori mesajul de
subscribe sau unsubscribe ori erorile care spun ce problema 
la mesaj exista.

Feedback: consider ca aceasta tema este foarte folositoare si
ajuta mult la intelegerea unei aplicatii client-server.
