import serial
import pygame
import tkinter as tk
from tkinter import ttk
import re
import time

# Set up the serial connection
ser = serial.Serial('COM8', 115200, timeout=0)  # Set timeout to 0 for non-blocking
data = ""
while data != "begin":
    if data != "":
        print(data)
    data = ser.readline().decode('utf-8').rstrip()  # read data from serial and decode it
# Initialize Pygame
pygame.init()

# Set up the fixed display resolution
fixed_width, fixed_height = 250, 340
scale = 2
# Set up the display
window_size = (fixed_width * scale, fixed_height * scale)  # Set the desired window size
screen = pygame.display.set_mode((fixed_width, fixed_height),flags = pygame.SCALED)
pygame.display.set_caption('Arduino Serial Communication')
clock = pygame.time.Clock()

# Create a black framebuffer
framebuffer = pygame.Surface((fixed_width, fixed_height))
framebuffer.fill((0, 0, 0))

buffer = ""

def draw_pixel(x, y, r, g, b):
    try:
        # Set the pixel color in the framebuffer
        framebuffer.set_at((x, y), (r, g, b))
    except ValueError as e:
        print(f"Error drawing pixel: {e}")

# Function to send "1" to COM8
def send_home_command():
    ser.write(b'1')
    time.sleep(1)
# Function to send "2" to COM8 (Scan command)
def send_scan_command():
    ser.write(b'2')
    time.sleep(1)

# Main loop
def main_loop():
    global buffer
    try:
        # Read data from the Arduino in a non-blocking manner
        data = ser.read(ser.in_waiting).decode()
        buffer += data
        # Check if a complete message is received
        if '$' in buffer:
            messages = buffer.split('$')

            # Process each complete message
            for msg in messages[:-1]:
                # Do something with the message
                print("Received message:", msg)
                process_message(msg + '$')
            # Update the buffer with the remaining incomplete message
            buffer = messages[-1]

    except serial.SerialTimeoutException:
        # Handle timeout exception
        pass

    # Update the display
    screen.blit(framebuffer, (0, 0))
    pygame.display.update()

    # Limit the frame rate
    clock.tick(30)

    # Schedule the main loop to run again
    root.after(10, main_loop)

# Function to process a complete message
def process_message(data):
    if data.startswith('x=') and data.endswith('$'):
        # Extract values of x, y, r, g, and b from the received data
        match = re.match(r'x=(\d+);y=(\d+);r=(\d+);g=(\d+);b=(\d+)', data)
        if match:
            x, y, r, g, b = map(int, match.groups())
            draw_pixel(x, y, r, g, b)
        else:
            print(f"Invalid data format: {data}")

# Tkinter GUI setup
root = tk.Tk()
root.title('Control Panel')

# Create Auto-home button
auto_home_button = ttk.Button(root, text='Auto-home', command=send_home_command)
auto_home_button.grid(row=0, column=1, padx=10, pady=10)

# Create Scan button
scan_button = ttk.Button(root, text='Scan', command=send_scan_command)
scan_button.grid(row=1, column=1, padx=10, pady=10)

# Schedule the main loop to run initially
root.after(10, main_loop)

# Run the Tkinter event loop
root.mainloop()

# Close the serial connection
ser.close()

# Close Pygame
pygame.quit()