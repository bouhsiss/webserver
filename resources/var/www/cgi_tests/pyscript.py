
import socket
# Get the hostname of the local machine
hostname = socket.gethostname()

# Get the IP address of the local machine
ip_address = socket.gethostbyname(hostname)



# Display the IP address with the color gradient
print(f"Your IP address is: {ip_address}")
