import requests
import csv
import time
from datetime import datetime

url = "http://192.168.65.176/data"


with open('data.csv', 'w', newline='') as fichier:
    writer = csv.writer(fichier)

    # écriture du nom des colonnes
    noms_colonnes = ["Date", "Temperature", "Soil moisture", "Air moisture"]
    writer.writerow(noms_colonnes)


    # lecture des valeurs

    while True:
        
        response = requests.get(url)
        data = response.json() 

        ligne = [
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            data["Temperature"],
            data["Soil moisture"],
            data["Air moisture"]
        ]

        writer.writerow(ligne)
        print("Ligne ajoutée :", ligne)

        time.sleep(3) #temps entre deux lectures

