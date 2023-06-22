import socket
import threading

def handle_client(client_socket, client_address):
    print(f"Connessione accettata da {client_address[0]}:{client_address[1]}")

    # Ricevi i dati inviati dal client
    data = client_socket.recv(1024)
    print(f"Dati ricevuti: {data.decode()}")

    # Esempio di elaborazione della richiesta
    response = process_request(data)

    # Invia la risposta al client
    client_socket.sendall(response.encode())

    # Chiudi la connessione con il client
    client_socket.close()
    print(f"Connessione chiusa con {client_address[0]}:{client_address[1]}")

def start_server():
    # Indirizzo IP del server e porta da ascoltare
    ip = ''  #qualunque server
    port = 8000

    # Crea un socket TCP/IP
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Collega il socket all'indirizzo IP e alla porta specificati
    server_socket.bind((ip, port))

    # Inizia ad ascoltare le connessioni in arrivo
    server_socket.listen(5)
    print(f"Server in ascolto su {ip}:{port}")

    while True:
        # Accetta una nuova connessione
        client_socket, client_address = server_socket.accept()

        # Avvia un thread separato per gestire la connessione del client
        client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address))
        client_thread.start()

def process_request(data):
    # Esempio di elaborazione della richiesta del client
    # In questo caso, restituiamo una semplice risposta di conferma
    return "Richiesta ricevuta correttamente!"

if __name__ == "__main__":
    start_server()
