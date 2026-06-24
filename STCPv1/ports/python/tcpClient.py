import socket
import utils
import stcpEllipticCodec
class TcpClient:

    def messageHandler(self, theSock: socket, message):
        msg = f"{self.messageCount}: {message}"
        return msg

    def defaultHandshakeHandler(self, theSock: socket, theEC: stcpEllipticCodec.StcpEllipticCodec):
        return False, False, False
    
    def __init__(self, host='127.0.0.1', port=8888, theMessageHandler=messageHandler, theHandshakeHandler=defaultHandshakeHandler):
        self.host = host
        self.port = port
        self.handshakeHandler = theHandshakeHandler
        self.msgHandler = theMessageHandler
        self.theSock = None
        self.messageCount = 0

    def connect(self):
        self.theSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.theSock.connect((self.host, self.port))
        print("Client, doing hanshake............");
        self.handshakeHandler(self.theSock)
        print("Client, doing AES ................");


    def send_message(self, messageIncomignPlain):
        self.theSock.sendall(messageIncomignPlain)

    def recv_message(self):
        return self.theSock.recv(9000)

            
if __name__ == "__main__":
    client = TcpClient()
    client.connect()
    while True:
        msg = input("Anna viesti lähetettäväksi: ")
        if msg.lower() == 'exit':
            break
        client.send_message(msg)
