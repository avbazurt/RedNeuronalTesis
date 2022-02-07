from scripts.PySerial import *
from time import sleep
import numpy as np
import threading
from scripts.RedNeuronal import RedNeuronal
from tqdm import tqdm
import json
import os
from sys import platform as _platform

def IngresarFloat(n):
    bandera = True
    while bandera:
        try:
            valor = float(input("X{}: ".format(n)))
            return valor
        except:
            print("Error")


def SendModel(ESP32, new_model):
    print("Iniciando proceso flasheado")
    parametros = lambda mensaje: str({
        "cmd": "now",
        "MAC": "98:F4:AB:1B:1E:A0",
        "msg": str(mensaje)
    })

    # Primer comando para iniciar el flasheo
    mensaje = {
        "cmd": "new_model",
        "len": len(new_model)
    }
    ESP32.WritePort(parametros(mensaje))
    sleep(2)

    print("Send Data")
    mensaje = lambda indice, valor: \
        {
            "cmd": "model",
            "indice": indice,
            "valor": valor.strip()
        }

    loop = tqdm(total=len(new_model), position=0, leave=False)

    for indice in range(len(new_model)):
        ESP32.WritePort(parametros(mensaje(indice, new_model[indice])))

        loop.set_description("Flasheando..".format(indice))
        loop.update(1)
        sleep(0.03)

    # Comando para flashear nuevo dato
    mensaje = {
        "cmd": "end_model",
    }
    ESP32.WritePort(parametros(mensaje))

    loop.close()


def HiloReadData(ESP32, NodeRed, RedNeuronal):
    while (True):
        texto = ESP32.ReadPort().strip()
        if (texto == "FlashModel"):
            #SendModel(ESP32, new_model)
            pass

        else:
            dic = json.loads(texto)
            try:
                if (dic["cmd"] == "muestra"):
                    NodeRed.WritePort(texto)
                    predic = RedNeuronal.Predicion(float(dic["FP3"]))

                    parametros = lambda mensaje: str({
                    "cmd": "now",
                    "MAC": "3C:61:05:13:81:34",
                    "msg": str(mensaje)
                    })

                    mensaje = {"cmd":"predict","valor":predic}
                    print(dic)
                    print(mensaje)
                    print("")
                    ESP32.WritePort(parametros(mensaje))
            except:
                pass



def HiloReadNodeRed(ESP32, NodeRed, new_model):
    while (True):
        texto = NodeRed.ReadPort().strip()
        SendModel(ESP32, new_model)


if __name__ == '__main__':
    # Limpiamos la consola
    os.system("clear")

    RedNeuronal = RedNeuronal()
    RedNeuronal.UpdateDatos()
    RedNeuronal.ConfigurarModelo()
    RedNeuronal.EntrenamientoModelo(1)

    # Definimos el sistema para los puertos seriales
    if _platform == "linux" or _platform == "linux2":
        print("# linux")
        port_NodeRed = "/dev/pts/1"
        port_Raspberry = "/dev/pts/2"
        port_ESP32 = "/dev/ttyAMA0"

    elif _platform == "win32" or _platform == "win64":
        print("# Windows")
        port_NodeRed = "COM20"
        port_Raspberry = "COM21"
        port_ESP32 = "COM22"

    # Realizamos una comunicacion entre dos puertos seriales
    NodeRed = RasberrySerial(port_NodeRed)
    ESP32 = RasberrySerial(port_ESP32)

    t1 = threading.Thread(target=HiloReadData, args=(ESP32, NodeRed,RedNeuronal), daemon=True)
    t1.start()

    #t2 = threading.Thread(target=HiloReadNodeRed, args=(ESP32, NodeRed, new_model), daemon=True)
    #t2.start()

    bandera = True

    while bandera:
        try:
            pass
        except:
            bandera = False





