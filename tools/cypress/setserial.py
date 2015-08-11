import sys
import os
import intelhex

bus = int(sys.argv[1])
devnum = int(sys.argv[2])
serial = int(sys.argv[3])

ih = intelhex.IntelHex("slave.hex")
ih.padding = 0xff
ih.start_addr = None

patch = 0x1ba;

print ("Programm EEPROM of Cypress at bus ",bus,", device number: ",devnum,", with serial number: ",serial);

ih[patch]=0x30+int(serial/10)
ih[patch+2]=0x30+int(serial%10)

sio=open("flyspi.hex",'w')
ih.tofile(sio,format='hex')
sio.close()

device="/dev/bus/usb/"+str(bus).zfill(3)+"/"+str(devnum).zfill(3)

os.system("sudo /sbin/fxload -I flyspi.hex -t fx2lp -s Vend_Ax.hex -D "+device+" -c 0x01 -v")
