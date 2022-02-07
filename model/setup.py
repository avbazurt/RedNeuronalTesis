from setuptools import setup
import os
import platform

with open('requirements.txt') as f:
    with open('requirements.txt') as f:
        requirements = f.readlines()

requirements = [dependencia.strip() for dependencia in requirements]

arch = platform.uname().machine
if arch == 'armv7l':
    tensorflow = "https://github.com/Qengineering/Tensorflow-Raspberry-Pi/raw/master/tensorflow-2.1.0-cp37-cp37m-linux_armv7l.whl"
    requirements.insert(0, tensorflow)

elif arch == 'AMD64':
    tensorflow = "tensorflow~=2.1.1"
    requirements.insert(0, tensorflow)
    requirements.pop(1)
    numpy_dep = "numpy~=1.19.5"
    requirements.insert(1, numpy_dep)


else:
    raise Exception(f'Could not find TensorFlow binary for target {arch}. Please open a Github issue.')


file = open("requirements.txt","w")
for dependencia in requirements:
    file.write(str(dependencia)+"\n")
file.close()


setup(
    name='model',
    version='0.1',
    url='https://github.com/avbazurt',
    author='vidalbazurto',
    author_email='avbazurt@espol.edu.ec',
    description='Tesis 2021',
    install_requires=requirements
)
