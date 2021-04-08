import threading
import imagezmq
import simplejpeg

# https://github.com/jeffbass/imagezmq/blob/master/docs/fast-pub-sub.rst#fast-pub-sub-subscriber-helper-class
class VideoStreamSubscriber:
    def __init__(self, connstr):
        self.connstr = connstr
        self._stop = False
        self._data_ready = threading.Event()
        self._thread = threading.Thread(target=self._run, args=())
        self._thread.daemon = True
        self._thread.start()

    def receive(self, timeout=5.0):
        while True:
            flag = self._data_ready.wait(timeout=timeout)
            if not flag:
                print(f"No data from {self.connstr} in {timeout}s")
            else:
                break
        self._data_ready.clear()
        return self._data

    def _run(self):
        receiver = imagezmq.ImageHub(self.connstr, REQ_REP=False)
        while not self._stop:
            data, jpg_buffer = receiver.recv_jpg()
            self._data = (data, simplejpeg.decode_jpeg( jpg_buffer, colorspace='BGR'))
            self._data_ready.set()
        receiver.close()

    def close(self):
        self._stop = True
