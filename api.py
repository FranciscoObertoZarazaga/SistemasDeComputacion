import requests

def getData():
    url = 'https://api.worldbank.org/v2/en/country/all/indicator/SI.POV.GINI?format=json&date=2011:2020&per_page=32500&page=1&country=%22Argentina%22'
    response = requests.get(url)
    if response.status_code:
        return response.json()
    else:
        print(response.status_code)
        print(response.text)
        print('Error al conectar con la API')
        exit()
