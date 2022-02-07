import pandas as pd
import mysql.connector
import sqlite3 as sql

mydb = mysql.connector.connect(
                    host="192.168.100.28",
                    user="avbazurt",
                    password="avbazurt",
                    database="TesisConsumoEnergetico")


df = pd.read_sql('SELECT * FROM mediciones',mydb)
df = df[df['FP3'].notna()]
df["FP3"][df["FP3"]<0.8] = 0.8
df = df[:][df["day"]<10]

mydb_lite = sql.connect("../dataBase/medidorTrifasico.db")
mycursor_lite = mydb_lite.cursor()

df.to_sql(name="mediciones", con=mydb_lite, if_exists = 'replace', index=False)



