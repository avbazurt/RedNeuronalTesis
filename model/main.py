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


def IngresarFloat(n):
    bandera = True
    while bandera:
        try:
            valor = float(input("X{}: ".format(n)))
            return valor
        except:
            print("Error")




if __name__ == '__main__':
    RasberrySerial = RasberrySerial("/dev/ttyAMA0")

    parametros = lambda mensaje: str({
        "cmd": "now",
        "MAC": "98:F4:AB:1B:21:88",
        "msg": str(mensaje)
    })

    RedNeuronal = RedNeuronal()
    RedNeuronal.UpdateDatos()
    RedNeuronal.ConfigurarModelo()
    RedNeuronal.EntrenamientoModelo(100)
    #
    new_model = RedNeuronal.GenerateLiteModel()

    menu = """Menu Principal:
    1) Cargar Modelo
    2) Predecir Resultado
    3) Exit
    """

    while (True):
        try:
            opcion = int(input(menu))

            if (opcion==1):
                try:
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
                except:
                    print("Se suspende envio datos")

            elif (opcion==2):
                list_predicion = []
                for i in range(8):
                    list_predicion.append(IngresarFloat(i))
                print(RedNeuronal.Predicion(list_predicion))
            elif (opcion ==3):
                break
        except:
            pass

