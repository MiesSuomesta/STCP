import socket

class TcpServer:

    def defaultConnectionHandler(connectionSocket, connectionAddress):
        print(f"TcpServer got connection: {connectionSocket}, {connectionAddress}")
        return None

    def defaultHandshakeHandler(self, theSock: socket):
        return False, False, False

    def __init__(self, host='127.0.0.1', port=8888, connectionHandler=defaultConnectionHandler, theHandshakeHandler=defaultHandshakeHandler):
        self.host = host
        self.port = port
        self.connHandler = connectionHandler
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.bind((self.host, self.port))
        self.server.listen()
        self.handshakeHandler = theHandshakeHandler
        print(f"Palvelin käynnistetty osoitteessa {self.host}:{self.port}")

    def start(self):
        while True:
            conn, addr = self.server.accept()
            with conn:
                print(f"Yhteys vastaanotettu osoitteesta {addr}...")
                # self, theSock: socket, theEC: stcpEllipticCodec.StcpEllipticCodec
                print("Server, doing hanshake............");
                self.handshakeHandler(conn)
                print("Server, doing AES ................");

                self.connHandler(conn, addr)

if __name__ == "__main__":
    server = TcpServer()
    server.start()
