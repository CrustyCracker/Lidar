import socket
import pygame
from include.map import Map
from include.constants import *

FPS = 60
WIN = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption('SUPER PROJEKT')

msgFromClient = "Hello UDP Server"
bytesToSend = str.encode(msgFromClient)
serverAddressPort = ("10.42.0.200", 8080)
bufferSize = 1024

# Create a UDP socket at client side
UDPClientSocket = socket.socket(
    family=socket.AF_INET, type=socket.SOCK_DGRAM)


def get_data():

    # Send to server using created UDP socket
    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    msgFromServer = UDPClientSocket.recvfrom(bufferSize)
    msg = int.from_bytes(msgFromServer[0], byteorder='little')

    msgFromServer = UDPClientSocket.recvfrom(bufferSize)
    msg2 = int.from_bytes(msgFromServer[0], byteorder='little') + 20
    print(f'Angle:{msg} distance:{msg2}')
    return(msg2, msg)



def main():

    run = True
    clock = pygame.time.Clock()
    map = Map(WIDTH, HEIGHT)
    WIN.fill(SCREEN_COLOR)
    while run:
        clock.tick(FPS)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            if event.type == pygame.KEYDOWN:
                # If pressed key is ESC quit program
                if event.key == pygame.K_SPACE:
                    WIN.fill(SCREEN_COLOR)
        line_data = get_data()
        map.draw(WIN, line_data)

        pygame.display.update()

    pygame.quit()


if __name__ == "__main__":
    main()
