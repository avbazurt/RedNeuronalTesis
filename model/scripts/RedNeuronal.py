from __future__ import absolute_import, division, print_function, unicode_literals
import pandas
import numpy as np
import tensorflow as tf
from tensorflow.keras import layers
from tensorflow.keras.utils import to_categorical
from keras.utils import np_utils

from tensorflow.keras.models import load_model

from sklearn.model_selection import train_test_split
from sklearn.datasets import make_circles

from keras import backend as K
import matplotlib.pyplot as plt
import re
import hexdump

class RedNeuronal:
    def __init__(self):
        self.model = None

        self.input_train = None
        self.input_test = None

        self.ouput_train = None
        self.ouput_test = None


    def UpdateDatos(self):
        kp = 1
        n1 = np.arange(0, 220, kp)
        n2 = np.arange(0, 220, kp)
        n3 = np.arange(0, 220, kp)
        n4 = np.arange(0, 220, kp)
        n5 = np.arange(0, 220, kp)
        n6 = np.arange(0, 220, kp)
        n7 = np.arange(0, 220, kp)
        n8 = np.arange(0, 220, kp)

        X = np.transpose([n1, n2, n3, n4, n5, n6, n7, n8])
        Y = np.random.randint(0, 2, size=[np.shape(X)[0], 3])

        input_train, input_test, ouput_train,ouput_test = train_test_split(X,Y,train_size=0.8,random_state=23)

        self.input_train = np.array(input_train).tolist()
        self.input_test = np.array(input_test).tolist()
        self.ouput_train = np.array(ouput_train).tolist()
        self.ouput_test = np.array(ouput_test).tolist()


    def ConfigurarModelo(self):
        self.model = tf.keras.Sequential()

        self.model.add(layers.Dense(32, activation='sigmoid', input_dim=8))
        self.model.add(layers.Dense(16, activation='relu'))
        self.model.add(layers.Dense(8, activation='relu'))
        self.model.add(layers.Dense(3, activation='sigmoid'))

        # Configulación del entrenamiento
        self.model.compile(loss='mse', optimizer='adam', metrics=['accuracy'])
        self.model.summary()


    def EntrenamientoModelo(self,epochs):
        self.model.fit(self.input_train, self.ouput_train, epochs=epochs, batch_size=1, verbose=True)
        loss, accuracy = self.model.evaluate(self.input_test, self.ouput_test, verbose=0)
        print("Test accuracy : " + str(accuracy))


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


    def Predicion(self,list_entrada):
        input_predic = np.array([list_entrada])

        return self.model.predict(input_predic)
