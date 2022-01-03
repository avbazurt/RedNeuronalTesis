from scripts.PySerial import *
from time import sleep
import threading
from scripts.RedNeuronal import RedNeuronal

def HiloReadData(RasberrySerial,bandera):
    while (bandera):
        texto = RasberrySerial.ReadPort()
        if ("Received message from:" in texto):
            texto = texto.replace("Received message from:","")
            print(texto)

def HiloWriteData(RasberrySerial,bandera):
    while (bandera):
        dic_data = {
            "cmd":"now",
            "MAC":"3C:61:05:13:81:34",
            "msg":"{'hola':'hola'}"
        }
        print(str(dic_data))
        RasberrySerial.WritePort(str(dic_data)+"\n")
        sleep(5)



RasberrySerial  = RasberrySerial("/dev/ttyUSB1")

parametros = lambda mensaje : str({
    "cmd": "now",
    "MAC": "3C:61:05:13:81:34",
    "msg": str(mensaje)
})



RedNeuronal = RedNeuronal()
RedNeuronal.UpdateDatos()
RedNeuronal.ConfigurarModelo()
RedNeuronal.EntrenamientoModelo(2)
#
new_model = RedNeuronal.GenerateLiteModel()

#Primer comando para iniciar el flasheo
mensaje = {
    "cmd":"new_model",
    "len":len(new_model)
    }
RasberrySerial.WritePort(parametros(mensaje))
sleep(2)

print("Send Data")
mensaje = lambda indice,valor: \
    {
    "cmd":"model",
    "indice":indice,
    "valor":valor.strip()
    }

for indice in range(len(new_model)):
    RasberrySerial.WritePort(parametros(mensaje(indice,new_model[indice])))
    sleep(0.03)


#Comando para flashear nuevo dato
mensaje = {
    "cmd":"end_model",
    }
RasberrySerial.WritePort(parametros(mensaje))



#hiloRead = threading.Thread(target=HiloReadData,args=(RasberrySerial,bandera,))
#hiloRead.start()

#hiloWrite = threading.Thread(target=HiloWriteData,args=(RasberrySerial,bandera,))
#hiloWrite.start()



#
# if __name__ == '__main__':
#     print('Running. Press CTRL-C to exit.')
#     with serial.Serial("/dev/ttyUSB1", 115200, timeout=1) as arduino:
#         time.sleep(0.1)  # wait for serial to open
#         if arduino.isOpen():
#             print("{} connected!".format(arduino.port))
#             try:
#                 while True:
#                     while arduino.inWaiting() == 0: pass
#                     if arduino.inWaiting() > 0:
#                         answer = arduino.readline()
#                         answer = str(answer,'utf-8')
#                         answer = answer.strip("\n")
#
#                         if ("Received message from:" in answer):
#                             answer = answer.replace("Received message from:","")
#                             print(answer)
#
#                             datos = answer.split("|")
#
#                             MAC = datos[0].strip()
#                             texto = datos[1].strip()
#
#                             print(MAC)
#
#
#
#
#                         arduino.flushInput()  # remove data after reading
#             except KeyboardInterrupt:
#                 print("KeyboardInterrupt has been caught.")
