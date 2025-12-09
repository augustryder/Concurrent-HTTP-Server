import socket
import sys

if __name__ == '__main__':

    route = sys.argv[1] if len(sys.argv) > 1 else '/'
    HOST = 'localhost'
    PORT = 8000
    MESSAGE = f'GET {route} HTTP/1.1'
    BUFFER_SIZE = 4096

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        sock.connect((HOST, PORT))
        print(f"Connected to {HOST}:{PORT}.")

        sock.sendall(MESSAGE.encode('utf-8'))
        print(f"Sent: {MESSAGE}\n")

        response = not None
        while (response):
            response = sock.recv(BUFFER_SIZE)
            if route == '/':
                print(f"{response.decode('utf-8')}")
            else:
                print(f"{response}")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()
        print("Connection closed.")


