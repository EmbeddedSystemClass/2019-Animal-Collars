#Build script for the Project GUI
#To freeze new version, enter "python SetupGui.py build" in the command line


import sys
from cx_Freeze import setup, Executable
import os

import os.path

PYTHON_INSTALL_DIR = os.path.dirname(os.path.dirname(os.__file__))
os.environ['TCL_LIBRARY'] = os.path.join(PYTHON_INSTALL_DIR, 'tcl', 'tcl8.6')
os.environ['TK_LIBRARY'] = os.path.join(PYTHON_INSTALL_DIR, 'tcl', 'tk8.6')

base = None
if sys.platform == 'win32':
    base = 'Win32GUI'

syspath = r"C:\Users\colli\AppData\Local\Programs\Python\Python37-32/DLLs"

executables = [
    Executable('CollarProgram.py', base=base)
]

buildOptions = dict(
    packages=[],
    excludes=[],
    include_files=[syspath + '/tcl86t.dll', syspath + '/tk86t.dll']
)


setup(name='AnimalTrackingCollarProgram',
      version='1.0',
      description='User Program to set up and control the 2019 animal tracking collar system.',
      executables=executables,
      options=dict(build_exe=buildOptions)

      )
