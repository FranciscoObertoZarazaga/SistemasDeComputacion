import matplotlib.pyplot as plt
import matplotlib.animation as animation
import paramiko

# Configuración de la conexión SSH
raspi_ip = '192.168.191.106'
raspi_user = 'fran'
raspi_password = 'fran12'

# Inicializar las listas para guardar las mediciones
temperatura_data = []
luz_data = []

# Crear las figuras y los ejes para la gráfica
fig, (ax1, ax2) = plt.subplots(2, 1)

# Crear la conexión SSH
ssh_client = paramiko.SSHClient()
ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh_client.connect(raspi_ip, username=raspi_user, password=raspi_password)

def obtener_medicion(comando):
    try:
        stdin, stdout, stderr = ssh_client.exec_command(comando)
        resultado = stdout.read().decode().strip()
        print(resultado)
        if resultado:  # Verificar si el resultado no está vacío
            return float(resultado)
        else:
            print("No se obtuvo ningún dato.")
            return None
    except Exception as e:
        print(f"Error obteniendo la medición: {e}")
        return None

def actualizar(i):
    # Obtener la última medición de temperatura
    temp = obtener_medicion("sudo tail -n 1 /dev/temperatura")
    if temp is not None:
        temperatura_data.append(temp)

    # Obtener la última medición de luz
    luz = obtener_medicion("sudo tail -n 1 /dev/luz")
    if luz is not None:
        luz_data.append(luz)

    # Limitar las listas para que solo contengan las últimas 50 mediciones
    temperatura_data_limited = temperatura_data[-50:]
    luz_data_limited = luz_data[-50:]

    # Limpiar los ejes
    ax1.clear()
    ax2.clear()

    # Graficar los datos
    ax1.plot(temperatura_data_limited, label='Temperatura (°C)')
    ax2.plot(luz_data_limited, label='Luz (lux)')

    # Configurar las etiquetas y leyendas
    ax1.set_ylabel('Temperatura (°C)')
    ax1.legend(loc='upper left')
    ax2.set_ylabel('Luz (lux)')
    ax2.legend(loc='upper left')
    ax2.set_xlabel('Tiempo (s)')


# Configurar la animación
ani = animation.FuncAnimation(fig, actualizar, interval=100)

# Mostrar la gráfica
plt.tight_layout()
plt.show()

# Cerrar la conexión SSH al finalizar
ssh_client.close()
