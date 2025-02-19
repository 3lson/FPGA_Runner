import subprocess
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from threading import Thread
from queue import Queue

# Path to your Nios II Command Shell batch file
NIOS_CMD_SHELL_BAT = "C:/intelFPGA_lite/18.1/nios2eds/Nios II Command Shell.bat"
TAPS = 49  # Number of filter taps

# Global lists to store accelerometer data
x_data = []
y_data = []
z_data = [] 

def collect():
    global x_data, y_data, z_data
    x_data.clear()  # Clear old data before a new collection
    y_data.clear()
    z_data.clear()

    # Launch the Nios II terminal process
    process = subprocess.Popen(
        ['nios2-terminal'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    print("Reading accelerometer data... Press Ctrl+C to stop.")

    try:
        while True:
            # Send 'r' to the process to trigger data collection
            process.stdin.write('r\n')
            process.stdin.flush()

            line = process.stdout.readline().strip()
            if "Accelerometer Data" in line:
                print(f"Received data: {line}")  # Debugging print

                try:
                    # Remove any trailing text after the Z value
                    line = line.split(" - ")[0]  # Keep only "Accelerometer Data: X=7, Y=-13, Z=236"
                   
                    parts = line.split(',')
                    if len(parts) < 3:
                        print(f"Unexpected format: {line}")
                        continue

                    # Extract values
                    x_value = int(parts[0].split('=')[1].strip())
                    y_value = int(parts[1].split('=')[1].strip())
                   
                    # Extract Z correctly
                    z_raw = parts[2].split('=')[1].strip()  # Get raw Z value
                    print(f"Raw Z Value: '{z_raw}'")  # Debugging

                    z_value = int(z_raw)  # Convert Z to int

                    # Store values
                    x_data.append(x_value)
                    y_data.append(y_value)
                    z_data.append(z_value)  # Now storing Z correctly!

                except Exception as e:
                    print(f"Error parsing line: {line}, {e}")
                    continue

            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\nData collection stopped.")

    finally:
        process.terminate()

def set_mode(mode):
    if mode not in ['0', '1']:
        raise ValueError("Mode must be '0' (unfiltered) or '1' (filtered)")
    process = subprocess.Popen(
        ['nios2-terminal'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    process.stdin.write(f"{mode}\n")
    process.stdin.flush()
    response = process.stdout.readline()
    print(f"Set mode to {mode}: {response.strip()}")
    process.terminate()

def send_coefficients(coeffs):
    print("Starting coefficient transmission ... ")
    process = subprocess.Popen(
        ['nios2-terminal'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    time.sleep(0.1)
   
    for i, coeff in enumerate(coeffs):
        coeff_str = f"{coeff:.4f}"
        print(f"Sending coefficient {i+1}/{len(coeffs)}: {coeff_str}")
        for char in coeff_str:
            process.stdin.write(char.encode())
            process.stdin.flush()
            time.sleep(0.01)
        process.stdin.write(','.encode())
        process.stdin.flush()
        time.sleep(0.01)
   
    print("Coefficients sent to hardware")
    process.terminate()

def plot_accelerometer_data():
    if not x_data or not y_data or not z_data:
        print("No accelerometer data available. Please run option 2 first to collect data.")
        return

    fig, ax = plt.subplots()
    ax.plot(x_data, label="X Data", color='r')
    ax.plot(y_data, label="Y Data", color='g')
    ax.plot(z_data, label="Z Data", color='b') 

    ax.set_title("Accelerometer Data (X,Y and Z)")
    ax.set_xlabel("Sample Number")
    ax.set_ylabel("Accelerometer Value")
    ax.legend()
    ax.grid(True)

    plt.show()


def main():
    while True:
        print("\nAccelerometer Data Reader")
        print("1. Set Mode")
        print("2. Read Data Once")
        print("3. Continuous Read")
        print("4. Update Filter Coefficients")
        print("5. Plot Accelerometer Data")
        print("6. Exit")
        choice = input("Enter your choice (1-6): ")

        if choice == '1':
            mode = input("Enter mode (0 for unfiltered, 1 for filtered): ")
            set_mode(mode)
        elif choice == '2':
            print("Reading accelerometer data...")
            collect()  # Single read action (can be modified as needed)
        elif choice == '3':
            collect()  # Start continuous reading loop
        elif choice == '4':
            new_coeffs = [float(x) for x in input("Enter new coefficients (comma-separated): ").split(',')]
            if len(new_coeffs) != TAPS:
                print(f"Error: Expected {TAPS} coefficients")
            else:
                send_coefficients(new_coeffs)
        elif choice == '5':
            plot_accelerometer_data()
        elif choice == '6':
            print("Exiting...")
            break
        else:
            print("Invalid choice. Please try again.")

        time.sleep(1)

if __name__ == "__main__":
    main()
