import serial
import os
import time



try:
    ser = serial.Serial('COM5', 9600)  # Remplacez 'COM5' par le port de votre Arduino

except :
    print("\nerror open COM")

code_lines = []
time.sleep(2)

# start extraction
command = "start_extraction\n".encode()
line = ser.write (command)
line = ser.readline().decode('utf-8').strip()
print (line)

while True:
    line = ser.readline().decode('utf-8')
    if (line.strip() == "Extraction completed."):
        print (line)
        break
    
    if line and not line.startswith("//"):
        code_lines.append(line.strip())
#or line == ""
# Écrire le code dans un fichier
desktop_path = os.path.expanduser(r"~sedak\Documents")  
file_path = os.path.join(desktop_path, "extracted_data.txt")
with open(file_path, "w") as code_file:
    for line in code_lines:
        code_file.write(line + '\n')  # Utilisez code_file.write ici

print("Code Arduino sauvegardé dans", file_path)

# Fermer le port série
ser.close()
