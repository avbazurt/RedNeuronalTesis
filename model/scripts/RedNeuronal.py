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
        X, Y = make_circles(n_samples=500, factor=0.5, noise=0.05)
        X = 5 * X

        input_train, input_test, ouput_train,ouput_test = train_test_split(X,Y,train_size=0.8,random_state=23)

        self.input_train = input_train
        self.input_test = input_test
        self.ouput_train = ouput_train
        self.ouput_test = ouput_test


    def ConfigurarModelo(self):
        self.model = tf.keras.Sequential()

        self.model.add(layers.Dense(16, activation='relu', input_dim=2))
        self.model.add(layers.Dense(8, activation='relu'))
        self.model.add(layers.Dense(1, activation='sigmoid'))

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


