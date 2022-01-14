import mysql.connector
from threading import Semaphore,Thread
from time import sleep
import numpy as np
import sys

class ServidorMySQL():
    def __init__(self,host,user,password,database):
      self.host = host
      self.user = user
      self.password = password
      self.database = database
      self.mydb = None
      self.mycursor = None
      self.control_threads = Semaphore(1)
      self.connect()
      self.disconect()


    def connect(self):
        while True:
            try:
                self.mydb = mysql.connector.connect(
                    host=self.host,
                    user=self.user,
                    password=self.password,
                    database=self.database)
                self.mycursor = self.mydb.cursor()
                return None
            except:
                print("Error conectar MySQL, reintentando en 5 segundos")
                sleep(5)

    def disconect(self):
        self.mycursor.close()
        self.mydb.close()


    def SELECT(self,query,type_dic = True):
        with self.control_threads:
            try:
                self.connect()
                self.mycursor.execute(query)

                field_names = [i[0] for i in self.mycursor.description]

                if (type_dic):
                    myresult = np.array(self.mycursor.fetchall())

                    dic = {}
                    for n in range(len(field_names)):
                        dic[field_names[n]] = list(myresult[:,n])
                    return dic

                else:
                    myresult = self.mycursor.fetchall()

                    l_resultado = []
                    for i in myresult:
                        l_resultado.append(list(i))

                    return [field_names,l_resultado]


            except:
                print("Unexpected error:", sys.exc_info()[0])
                if (type_dic):
                    return {}
                else:
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
