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







if __name__ == '__main__':
    RasberrySerial = RasberrySerial("/dev/ttyAMA0")

    parametros = lambda mensaje: str({
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

    while (True):
        input("Presione Enter para iniciar")

        # Primer comando para iniciar el flasheo
        mensaje = {
            "cmd": "new_model",
            "len": len(new_model)
        }
        RasberrySerial.WritePort(parametros(mensaje))
        sleep(2)

        print("Send Data")
        mensaje = lambda indice, valor: \
            {
                "cmd": "model",
                "indice": indice,
                "valor": valor.strip()
            }

        for indice in range(len(new_model)):
            RasberrySerial.WritePort(parametros(mensaje(indice, new_model[indice])))
            sleep(0.03)

        # Comando para flashear nuevo dato
        mensaje = {
            "cmd": "end_model",
        }
        RasberrySerial.WritePort(parametros(mensaje))


