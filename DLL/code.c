#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Errore durante la creazione della socket" << std::endl;
        return 1;
    }

    // Impostazione dei parametri della socket server
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8000); // Porta 8000
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Indirizzo IP 127.0.0.1 (localhost)

    // Connessione al server
    if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Errore durante la connessione al server" << std::endl;
        return 1;
    }

    // Invio del byte "x"
    char data = 'x';
    if (send(sockfd, &data, sizeof(data), 0) < 0) {
        std::cerr << "Errore durante l'invio dei dati" << std::endl;
        return 1;
    }

    std::cout << "Byte inviato con successo" << std::endl;

    // Chiusura della socket
    close(sockfd);

    return 0;
}