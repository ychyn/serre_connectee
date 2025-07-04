import requests
import csv
import time
from datetime import datetime

url = "http://192.168.160.176/data"

# Ouvre le fichier CSV en écriture
with open('data.csv', 'w', newline='') as fichier:
    writer = csv.writer(fichier)

    # Écriture de l'en-tête
    noms_colonnes = ["Date", "Temperature", "Soil moisture", "Air moisture", "Luminosite"]
    writer.writerow(noms_colonnes)

    # Boucle de récupération
    while True:
        try:
            response = requests.get(url, timeout=5)
            response.raise_for_status()  # Lève une exception si réponse != 200

            data = response.json()

            ligne = [
                datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                data.get("temperature", "N/A"),
                data.get("soil moisture", "N/A"),
                data.get("air moisture", "N/A"),
                data.get("luminosite", "N/A")
            ]

            writer.writerow(ligne)
            fichier.flush()  # Écrit immédiatement les données dans le fichier
            print("Ligne ajoutée :", ligne)

        except requests.exceptions.RequestException as e:
            print(f"[ERREUR] Requête échouée : {e}")
        except ValueError:
            print("[ERREUR] Réponse JSON invalide")
        except Exception as e:
            print(f"[ERREUR] Inattendue : {e}")

        time.sleep(3)
