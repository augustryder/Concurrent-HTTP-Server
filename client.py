import socket

if __name__ == '__main__':
    # Example usage
    HOST = 'localhost'
    PORT = 8000
    MESSAGE = 'Hello, Server!'

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        sock.connect((HOST, PORT))
        print(f"Connected to {HOST}:{PORT}")

        sock.send(MESSAGE.encode('utf-8'))
        print(f"Sent: {MESSAGE}")

        response = sock.recv(100)
        print(f"Received: {response.decode('utf-8')}")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()
        print("Connection closed")


