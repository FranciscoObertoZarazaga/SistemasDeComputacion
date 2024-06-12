import matplotlib.pyplot as plt
import matplotlib.animation as animation
import paramiko

# SSH Connection Configuration
raspi_ip = '192.168.191.106'
raspi_user = 'fran'
raspi_password = 'fran12'

# Lists initialization to store measures
temperatura_data = []
luz_data = []

# Create figures and axes for the plot
fig, (ax1, ax2) = plt.subplots(2, 1)

# Make the SSH connection
ssh_client = paramiko.SSHClient()
ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh_client.connect(raspi_ip, username=raspi_user, password=raspi_password)

def obtener_medicion(comando):
    try:
        stdin, stdout, stderr = ssh_client.exec_command(comando)
        resultado = stdout.read().decode().strip()
        print(resultado)
        if resultado:  # Verifies if the result isn't empty
            return float(resultado)
        else:
            print("No data obtained.")
            return None
    except Exception as e:
        print(f"Error obtaining the measurement: {e}")
        return None

def actualizar(i):
    # Get last temperature measurement 
    temp = obtener_medicion("sudo tail -n 1 /dev/temperatura")
    if temp is not None:
        temperatura_data.append(temp)

    # Get last light measurement 
    luz = obtener_medicion("sudo tail -n 1 /dev/luz")
    if luz is not None:
        luz_data.append(luz)

    # Limit the lists so they only store the latest 50 measurements
    temperatura_data_limited = temperatura_data[-50:]
    luz_data_limited = luz_data[-50:]

    # Clear the axis
    ax1.clear()
    ax2.clear()

    # Plot data
    ax1.plot(temperatura_data_limited, label='Temperature (°C)')
    ax2.plot(luz_data_limited, label='Light (lux)')

    # Labels and legends configuration
    ax1.set_ylabel('Temperature (°C)')
    ax1.legend(loc='upper left')
    ax2.set_ylabel('Light (lux)')
    ax2.legend(loc='upper left')
    ax2.set_xlabel('Time (s)')


# Animation configuration
ani = animation.FuncAnimation(fig, actualizar, interval=100)

# Show plot
plt.tight_layout()
plt.show()

# At the end, close the SSH connection
ssh_client.close()
