import os
import ctypes, ctypes.util

class Sanitizer(object):
    @classmethod
    def library_path(cls):
        directory = os.path.abspath(os.path.dirname(__file__))
        return os.path.join(directory, 'libsanitize.so')

    libc = None
    libsanitize = None

    @classmethod
    def load_libraries(cls):
        if cls.libc and cls.libsanitize:
            return

        cls.libc = ctypes.CDLL(ctypes.util.find_library('libc'))

        cls.libc.free.argtypes = [ctypes.c_void_p]
        cls.libc.free.restype = None

        cls.libsanitize = ctypes.cdll.LoadLibrary(cls.library_path())

        cls.libsanitize.mode_load.argtypes = [ctypes.c_char_p]
        cls.libsanitize.mode_load.restype = ctypes.c_void_p

        cls.libsanitize.mode_free.argtypes = [ctypes.c_void_p]
        cls.libsanitize.mode_free.restype = None

        cls.libsanitize.sanitize.argtypes = [ctypes.c_char_p, ctypes.c_void_p]
        cls.libsanitize.sanitize.restype = ctypes.POINTER(ctypes.c_char)

    def __init__(self, path):
        self.load_libraries()
        self.mode = self.libsanitize.mode_load(path)

    def __del__(self):
        if self.mode:
            self.libsanitize.mode_free(self.mode)
            self.mode = None

    def sanitize(self, s):
        if not s:
            return s
        c = self.libsanitize.sanitize(s.encode('utf-8'), self.mode)
        r = ctypes.cast(c, ctypes.c_char_p).value
        self.libc.free(c)
        if r:
            return r.decode('utf-8')
        else:
            return r

if __name__ == '__main__':
    directory = os.path.abspath(os.path.dirname(__file__))
    mode_path = os.path.join(directory, 'modes/basic.xml')

    sanitizer = Sanitizer(mode_path)

    print sanitizer.sanitize(u'<div>Hello, <aside>World</aside></div>')

