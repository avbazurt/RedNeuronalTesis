#import serial, time
from serial import Serial

class RasberrySerial:
    def __init__(self,port):
        self.serial =  Serial(port, 115200, timeout=1)
        while not(self.serial.isOpen()):
            print("Esperando")
        print("Puerto abierto")


    def WritePort(self,text):
        text+="\n"
        comandoBytes = text.encode()
        self.serial.write(comandoBytes)

    def ReadPort(self):
        while self.serial.inWaiting() == 0: pass
        if self.serial.inWaiting() > 0:
            answer = self.serial.readline()
            answer = str(answer,'utf-8')
            answer = answer.strip("\n")
            return answer



