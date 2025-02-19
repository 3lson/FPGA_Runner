import subprocess
import time
import socket

NIOS_CMD_SHELL_BAT = "C:/intelFPGA_lite/18.1/nios2eds/Nios II Command Shell.bat"
TAPS = 49  

# Set up network connection to Mac
MAC_IP = "192.168.0.12"  # Replace with your Mac’s IP
PORT = 5005

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((MAC_IP, PORT))
print(f"Connected to Mac at {MAC_IP}:{PORT}")

def collect():
    process = subprocess.Popen(
        ['nios2-terminal'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    print("Reading accelerometer data... Sending to Mac...")

    try:
        while True:
            process.stdin.write('r\n')
            process.stdin.flush()

            line = process.stdout.readline().strip()
            if "Accelerometer Data" in line:
                try:
                    # Remove any trailing text after 'Z='
                    line = line.split(" - ")[0]  # Keeps only "Accelerometer Data: X=3, Y=-14, Z=238"
                   
                    parts = line.split(',')
                    x_value = int(parts[0].split('=')[1].strip())
                    y_value = int(parts[1].split('=')[1].strip())
                   
                    # Fix for Z-value extraction
                    z_value = int(parts[2].split('=')[1].strip().split()[0])

                    # Format and send data to Mac
                    data_str = f"{x_value},{y_value},{z_value}\n"
                    client.sendall(data_str.encode('utf-8'))
                    print(f"Sent: {data_str.strip()}")  # Debugging
                except Exception as e:
                    print(f"Error parsing line: {line}, {e}")

            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nData collection stopped.")
    finally:
        process.terminate()
        client.close()

if __name__ == "__main__":
    collect()
