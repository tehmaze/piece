import os

DEFAULT_PATH = '/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/opt/bin'

def which(binary):
    for path in os.environ.get('PATH', DEFAULT_PATH).split(':'):
        full = os.path.join(path, binary)
        if os.path.isfile(full) and os.access(full, os.X_OK):
            return full
