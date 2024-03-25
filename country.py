import pandas as pd
import matplotlib.pyplot as plt

class Country:
    
    def __init__(self, data):
        self.data = [data]
        self.name = data['country']['value']
        self.values = list()
        self.dataframe = None
        self.append(data)

    def append(self, data):
        date = data['date']
        value = data['value']
        dictionary = {'date': date, 'value': value}
        self.values.append(dictionary)
        self.dataframe = pd.DataFrame(self.values)
        self.dataframe['value'] = self.dataframe['value'] / 100 + 1

    def plot(self):
        df = self.dataframe.copy()
        df.dropna(inplace=True)
        df.sort_values('date', inplace=True)
        # Graficando
        plt.figure(figsize=(10, 6))  # Tamaño del gráfico
        plt.plot(df['date'], df['value'], marker='o', linestyle='-', color='blue')  # Gráfico de línea
        plt.title(f'Índice Gini a lo largo del tiempo para {self.name}')  # Título
        plt.xlabel('Año')  # Etiqueta del eje x
        plt.ylabel('Índice Gini')  # Etiqueta del eje y
        plt.grid(True)  # Añadir cuadrícula
        plt.show()
        return plt

    def __str__(self):
        return str(self.dataframe)
        