from api import getData
from country import Country
from time import sleep
from threading import Thread

response = getData()
countrys = dict()

for country in response[1]:
    countryName = country['country']['value']
    if countryName in countrys.keys():
        countrys[countryName].append(country)
    else:
        country = Country(country)
        countrys[country.name] = country

argentina = countrys['Argentina']
argentina.plot()