import socket
import os
import RPi.GPIO as GPIO 

# Configuring the socket to connect to
mysocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mysocket.bind(('', 22222))
okMessage = "HTTP/1.1 200 OK\n\n"

# Setting up GPIO pins
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
outputs = [6, 26, 12, 21, 4, 13, 20]
GPIO.setup(outputs, GPIO.OUT)


mysocket.listen()

while True:
    connect, addr = mysocket.accept()
    pid = os.fork()
    if (pid == 0):
        while True:
            request = connect.recv(1024)
            if not request:
                print('Client Disconnected')
                break
            print('Recieved: ' + request.decode("utf-8") + ' From: ' + addr[0] + ' Port: ' + str(addr[1]))
            # Parse file request 
            word = request.decode("utf-8")
            wordParse = word.split(" ")
            filename = wordParse[1]
            filename = filename[1:]
            print("Filename: " + filename)

            # Check path and enable appropriate LED
            if (filename == "index.html"):
                file = open('index.html', 'rb')
                filedata = file.read()
                data = okMessage.encode("utf-8") + filedata
                connect.send(data)
            elif (filename == "GreenOn"):
                GPIO.output(outputs[0:2], GPIO.LOW)
            elif (filename == "YellowOn"):
                GPIO.output(outputs[2:5], GPIO.LOW)
            elif (filename == "RedOn"):
                GPIO.output(outputs[5:7], GPIO.LOW)
            elif (filename == "AllOff"):
                GPIO.output(outputs, GPIO.HIGH)