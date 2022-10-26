import serial
import threading
#set serial as USB\VID_1366&PID_0101\000683133530

ser = serial.Serial('COM3', 115200, timeout=1)

def escrever():
    global ser
    while True:
        print("Lendo:")
        userIn = input()
        print("Lido:", userIn)
        ser.write(userIn.encode())
        line = ser.read()
        if line:
            print(line)


def ler():
    global ser
    while True:
        line = ser.read_until()
        if line:
            print(line)
    

t1 = threading.Thread(target=escrever)
#t2 = threading.Thread(target=ler)

t1.start()
#t2.start()