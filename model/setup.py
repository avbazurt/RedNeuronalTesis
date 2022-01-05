from setuptools import setup
import os

with open('requirements.txt') as f:
    with open('requirements.txt') as f:
        requirements = f.readlines()

requirements = [dependencia.strip() for dependencia in requirements]

arch = os.uname().machine
if arch == 'aarch64':
    tensorflow = "https://github.com/Qengineering/Tensorflow-Raspberry-Pi/raw/master/tensorflow-2.1.0-cp37-cp37m-linux_armv7l.whl"
    requirements.insert(0, tensorflow)

elif arch == 'x86_64':
    tensorflow = "tensorflow~=2.1.1"
    requirements.insert(0, tensorflow)

else:
    raise Exception(f'Could not find TensorFlow binary for target {arch}. Please open a Github issue.')


setup(
    name='model',
    version='0.1',
    url='https://github.com/avbazurt',
    author='vidalbazurto',
    author_email='avbazurt@espol.edu.ec',
    description='Tesis 2021'
)
