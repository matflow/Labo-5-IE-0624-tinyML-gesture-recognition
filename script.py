import serial
import csv

PuertoSerial = serial.Serial(port = '/dev/ttyACM0', baudrate=115200, timeout=1) 
print("Connected")
data_rows = []
header = ['x', 'y', 'z']
# abrir o crear archivo en modo escritura
f = open('leftright.csv', 'w', encoding='UTF8')  # nombre de archivo = gesto a grabar
# create the csv writer
writer = csv.writer(f)
print(header)
writer.writerow(header)  # escribe encabezado

SAMPLES_PER_GESTURE = 2000
counter = 0

while(counter < SAMPLES_PER_GESTURE):
#while():
    
    input = PuertoSerial.readline().decode('utf-8').replace('\r', "").replace('\n', "")
    input = input.split('\t')
    #print(input[2])
    if len(input) == 3:   # avoids missing axes on first reading
        writer.writerow(input)
        print(input)
    counter+=1



    

