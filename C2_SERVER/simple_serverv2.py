import socket
import threading
import sys
import time 

ATK_ADDR = '127.0.0.1'
ATK_PORT = 5000
clients = []

class ClientThread(threading.Thread):
    def __init__(self, client_socket, client_address, index):
        threading.Thread.__init__(self)
        self.client_socket = client_socket
        self.client_address = client_address
        self.timestamp = time.time()
        self.index = index

    def run(self):
        while True:
            try:
                data = self.client_socket.recv(1024).decode('utf-8')
                if not data:
                    break
                print(f"Received from {self.client_address}: {data}")
            except ConnectionResetError as e:
                print(e)
                for client in clients:
                    if client.index == self.index:
                        clients.remove(client)
                        print('Client removed')
                sys.exit(0) # kill thread?
        self.client_socket.close()

def send_to_all_clients(message, clients):
    for client in clients:
        client.client_socket.sendall(message.encode('utf-8'))

def showList(clients):
    if len(clients) == 0:
        print("Lista vuota")
    for client in clients:
        elapsed_time = int(time.time()-client.timestamp)
        print(f"{client.index}\t{client.client_address[0]}:{client.client_address[1]}\t{elapsed_time:.2f}s")

def handle_input(clients):
    while True:
        message = input("Enter message to send to all clients (Q to quit):\n")
        if message.upper() == 'Q':
            break
        elif message.upper() == "LIST":
            showList(clients)
        else:
            send_to_all_clients(message, clients)

    # Closing all client sockets
    for client in clients:
        client.client_socket.close()

    # Exiting the script
    sys.exit(0)

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((ATK_ADDR,ATK_PORT))
    server_socket.listen(5)

    

    input_thread = threading.Thread(target=handle_input, args=(clients,))
    input_thread.start()
    index = 0
    while True:
        client_socket, client_address = server_socket.accept()
        print(f"New connection from {client_address}")

        client_thread = ClientThread(client_socket, client_address, index)
        client_thread.start()

        clients.append(client_thread)

    server_socket.close()

if __name__ == '__main__':
    main()
