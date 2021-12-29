from __future__ import absolute_import, division, print_function, unicode_literals
import pandas
import numpy as np
import tensorflow as tf
from tensorflow.keras import layers
from tensorflow.keras.utils import to_categorical
from keras.utils import np_utils
from sklearn.model_selection import train_test_split
from keras import backend as K
from sklearn.datasets import make_circles
import matplotlib.pyplot as plt

from scripts.RedNeuronal import port

print("TensorFlow version: {}".format(tf.__version__))

X , Y  = make_circles(n_samples=500, factor=0.5, noise=0.05)
X = 5*X
X_train,X_test,y_train,y_test=train_test_split(X,Y,train_size=0.8,random_state=23)


X_train_1 = K.cast_to_floatx (X_train)
X_test_1 = K.cast_to_floatx (X_test)
y_train_1 = K.cast_to_floatx (y_train)

model= tf.keras.Sequential()
model.add(layers.Dense(16, activation='relu', input_dim=2))
model.add(layers.Dense(8, activation='relu'))
model.add(layers.Dense(1, activation='sigmoid'))

# Configulación del entrenamiento
model.compile(loss='mse', optimizer='adam', metrics=['accuracy'])
model.summary()





historial = model.fit(X_train_1,y_train_1, epochs=100,batch_size=1,verbose=True)



text = port(model,variable_name="mi_data")
print(text)