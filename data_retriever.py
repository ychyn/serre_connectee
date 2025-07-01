import requests
import json

url = "http://192.168.65.176/data"
r = requests.get(url)
cont = r.json()
print(type(cont))
