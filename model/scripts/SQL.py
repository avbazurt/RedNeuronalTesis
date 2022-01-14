import sqlite3 as sql
from threading import Semaphore
from time import sleep
import sys

class ServidorMySQL():
    def __init__(self,database):
      self.database = database
      self.mydb = None
      self.mycursor = None
      self.control_threads = Semaphore(1)
      self.connect()
      self.disconect()

    def connect(self):
        while True:
            try:
                self.mydb = sql.connect(self.database)
                #print("Conexion exitosa")
                self.mycursor = self.mydb.cursor()
                return None
            except:
                print("Error conectar MySQL, reintentando en 5 segundos")
                sleep(5)

    def disconect(self):
        self.mycursor.close()
        self.mydb.close()


    def SELECT(self,query):
        with self.control_threads:
            try:
                self.connect()
                self.mycursor.execute(query)
                field_names = [i[0] for i in self.mycursor.description]
                myresult = self.mycursor.fetchall()

                l_resultado = []
                for i in myresult:
                    l_resultado.append(list(i))
                return [field_names,l_resultado]


            except:
                print("Unexpected error:", sys.exc_info()[0])
                return [[],[]]


    def UPDATE(self,query,externo=True):
        with self.control_threads:
            self.connect()
            self.mycursor.execute(query)
            self.mydb.commit()
            self.disconect()

    def ADD_FILA(self, table, etiquetas, valores):
        with self.control_threads:
            self.connect()
            text_etiqueta = ""
            text_values = ""
            for i in etiquetas:
                text_etiqueta += str(i) + ", "
                text_values += "%s, "
            text_etiqueta = text_etiqueta.strip(", ")
            text_values = text_values.strip(", ")

            sql = "INSERT INTO {} ({}) VALUES ({})".format(table, text_etiqueta, text_values)

            if (isinstance(valores[0], tuple)) or (isinstance(valores[0], list)):
                print("Varios Datos")
                self.mycursor.executemany(sql, valores)
            else:
                print("Dato unico")
                self.mycursor.execute(sql, valores)

            self.mydb.commit()
            self.disconect()
            print("Datos agregados\n")