import requests
import json
import time
import csv

url = "http://192.168.65.176/data"


with open('data.csv', 'w', newline='') as fichier:
        writer = csv.writer(fichier)

noms_colonnes=["Date","Time","Temperature","Soil moisture","Air moisture"]


while True :
    r = requests.get(url)
    cont = r.json()
    time.sleep(3)
    data = json.load(cont)

    ligne=[data["Date"],data["Time"],data["Temperature"],data["Soil moisture"],data["Air moisture"]] # à raccorder avec le code arduino
    
    writer.writerow(ligne)
    