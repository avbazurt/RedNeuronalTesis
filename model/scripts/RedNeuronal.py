from __future__ import absolute_import, division, print_function, unicode_literals
import numpy as np

from tensorflow import math as tf_math
reduce_prod = tf_math.reduce_prod

import tensorflow as tf
from tensorflow.keras.layers import Dense, LSTM, Dropout, Input, Lambda, Dense, concatenate, Flatten
from scripts.SQL import ServidorMySQL

import hexdump

class RedNeuronal:
    def __init__(self):
        self.model = None

        self.input_train = None
        self.input_test = None

        self.ouput_train = None
        self.ouput_test = None

        self.timeSteps = 40
        self.list_input = list(np.ones(self.timeSteps))

    def UpdateDatos(self):
        dtb = ServidorMySQL("dataBase/medidorTrifasico.db")
        select_dtb = dtb.SELECT("SELECT FP3 FROM mediciones")

        fp3_train = []
        [fp3_train.append(x[0]) for x in select_dtb[1]]


        num_train = round(len(fp3_train) * 0.70)

        xTrain = []
        yTrain = []

        for i in range(0, num_train - self.timeSteps):
            # Lista - [FP3-actual]
            fp3 = fp3_train[i:i + self.timeSteps]

            train = list(fp3)
            xTrain.append(train)

            yTrain.append(fp3_train[i + self.timeSteps])

        xTrain = np.array(xTrain)
        xTrain = np.reshape(xTrain, (xTrain.shape[0], xTrain.shape[1], 1))

        self.input_train = np.array(xTrain).tolist()
        self.ouput_train = np.array(yTrain).tolist()



    def ConfigurarModelo(self):
        self.model = tf.keras.Sequential()

        """ capa 1 """
        self.model.add(LSTM(units=50, input_shape=(self.timeSteps, 1)))

        """ capa output """
        self.model.add(Dense(units=1))

        self.model.compile(optimizer='rmsprop', loss='mse',
                         metrics=[tf.keras.metrics.Accuracy(name='accuracy', dtype=None)])  # mse = mean_squared_error

        self.model.summary()


    def EntrenamientoModelo(self,epochs):
        self.model.fit(self.input_train, self.ouput_train, epochs=epochs, batch_size=1, verbose=True)
        #loss, accuracy = self.model.evaluate(self.input_test, self.ouput_test, verbose=0)
        #print("Test accuracy : " + str(accuracy))


    def GenerateLiteModel(self,optimize=False):
        converter = tf.lite.TFLiteConverter.from_keras_model(self.model)
        if optimize:
            if isinstance(optimize, bool):
                optimizers = [tf.lite.Optimize.OPTIMIZE_FOR_SIZE]
            else:
                optimizers = optimize
            converter.optimizations = optimizers
        tflite_model = converter.convert()
        bytes = hexdump.dump(tflite_model).split(' ')
        c_array = ', '.join(['0x%02x' % int(byte, 16) for byte in bytes])

        return c_array.split(",")


    def Predicion(self,dato):
        self.list_input.pop(0)
        self.list_input.insert(self.timeSteps-1,dato)

        print(self.list_input)

        input_predic = np.array([self.list_input])
        input_predic = np.reshape(input_predic, (input_predic.shape[0], input_predic.shape[1], 1))

        return self.model.predict(input_predic)[0][0]
