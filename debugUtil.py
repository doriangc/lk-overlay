import serial
try:
    from cStringIO import StringIO
except:
    from StringIO import StringIO
from xmodem import XMODEM, CRC
from time import sleep

def readUntil(char = None):
    def serialPortReader():
        while True:
            tmp = port.read(1)
            if not tmp or (char and char == tmp):
                break
            yield tmp
    return ''.join(serialPortReader())

def getc(size, timeout=1):
    return port.read(size)

def putc(data, timeout=1):
    port.write(data)
    sleep(0.001) # give device time to send ACK


port = serial.Serial(port='COM5',parity=serial.PARITY_NONE,bytesize=serial.EIGHTBITS,stopbits=serial.STOPBITS_ONE,timeout=0,xonxoff=0,rtscts=0,dsrdtr=0,baudrate=115200)
port.write("command that loads data via xmodem\r\n")
sleep(0.02) # give device time to handle command
readUntil(CRC)
buffer = StringIO('data to send')
XMODEM(getc, putc).send(buffer, quiet = 1)
buffer.close()
readUntil()