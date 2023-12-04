import serial
from time import sleep
# Define the COM port and baud rate
com_port = 'COM8'
baud_rate = 9600

# Open the serial port
ser = serial.Serial(com_port, baud_rate, timeout=1)
sleep(3)
# Create a buffer to store received data
buffer = ""

try:
    ser.write(b'2')
    while True:
        # Read a chunk of data from the serial port
        data = ser.read(ser.in_waiting or 1).decode('utf-8')

        # Append the data to the buffer
        buffer += data

        # Check if a complete message is received
        if '$' in buffer:
            messages = buffer.split('$')

            # Process each complete message
            for msg in messages[:-1]:
                # Do something with the message
                print("Received message:", msg)

            # Update the buffer with the remaining incomplete message
            buffer = messages[-1]

except KeyboardInterrupt:
    # Close the serial port on keyboard interrupt
    ser.close()