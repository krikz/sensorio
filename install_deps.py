import os
from SCons.Script import Import, DefaultEnvironment
import subprocess

env = DefaultEnvironment()
project_dir = env['PROJECT_DIR']

def install_requirements():
    requirements_path = os.path.join(project_dir, 'requirements.txt')
    subprocess.check_call([env['PYTHONEXE'], '-m', 'pip', 'install', '-r', requirements_path])

install_requirements()