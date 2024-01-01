import pygame
import tkinter as tk
from tkinter import ttk
import threading
from collections import deque

pygame.init()

# Set up the fixed display resolution
fixed_width, fixed_height = 10, 10
screen = pygame.display.set_mode((fixed_width, fixed_height), flags=pygame.SCALED)
pygame.display.set_caption('Arduino Serial Communication')
clock = pygame.time.Clock()

# Create a black framebuffer
framebuffer = pygame.Surface((fixed_width, fixed_height))
framebuffer.fill((0, 0, 0))

# Define the arrays
map_array = [[(0, 0, 0) for _ in range(fixed_width)] for _ in range(fixed_height)]
search_array = [[(0, 0, 0) for _ in range(fixed_width)] for _ in range(fixed_height)]

# Global variables for BFS
bfs_queue = deque()
visited = set()
search_active = False
display_search = False

# Function to handle mouse clicks
def handle_mouse_click(pos):
    x, y = int(pos[0]), int(pos[1])
    if x < fixed_width and y < fixed_height:
        map_array[y][x] = (255, 0, 0)  # Set pixel to red in map_array
        print(f"Pixel set at ({x}, {y})")  # Debug print

# Function to draw a pixel
def draw_pixel(x, y, r, g, b):
    try:
        framebuffer.set_at((x, y), (r, g, b))
    except ValueError as e:
        print(f"Error drawing pixel: {e}")

# Function to check if a position is valid
def is_valid(x, y):
    return 0 <= x < fixed_width and 0 <= y < fixed_height

# Function to start pixel finding (BFS initialization)
def start_pixel_finding():
    global bfs_queue, search_active, display_search, visited

    search_array = [[(0, 0, 0) for _ in range(fixed_width)] for _ in range(fixed_height)]
    display_search = True
    search_active = True
    visited.clear()

    start_x, start_y = fixed_width // 2, fixed_height // 2
    bfs_queue.append((start_x, start_y))
    visited.add((start_x, start_y))

def bfs_step():
    global bfs_queue, search_active

    if not bfs_queue:
        search_active = False
        print("Finished searching for connected red pixels.")
        return

    x, y = bfs_queue.popleft()

    if map_array[y][x] == (255, 0, 0):
        # Mark red pixel as green (found)
        search_array[y][x] = (0, 255, 0)

        # Add adjacent pixels to the queue
        directions = [(0, -1), (1, 0), (0, 1), (-1, 0)]
        for dx, dy in directions:
            nx, ny = x + dx, y + dy
            if is_valid(nx, ny) and (nx, ny) not in visited:
                bfs_queue.append((nx, ny))
                visited.add((nx, ny))
    elif (x, y) in visited:
        # Mark non-red pixel as white (visited)
        search_array[y][x] = (255, 255, 255)


# Main loop
def main_loop():
    global display_search
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                handle_mouse_click(pygame.mouse.get_pos())

        if search_active:
            bfs_step()  # Perform one step of BFS

        # Display the correct array
        current_array = search_array if display_search else map_array
        for y in range(fixed_height):
            for x in range(fixed_width):
                color = current_array[y][x]
                framebuffer.set_at((x, y), color)

        screen.blit(framebuffer, (0, 0))
        pygame.display.update()
        clock.tick(10)  # Control the frame rate

    pygame.quit()

# GUI setup
def create_gui():
    root = tk.Tk()
    save_button = ttk.Button(root, text="Save", command=lambda: None)  # Define your save logic here
    save_button.pack()
    start_button = ttk.Button(root, text="Start", command=start_pixel_finding)
    start_button.pack()
    root.mainloop()

# Start the application
if __name__ == "__main__":
    create_gui_thread = threading.Thread(target=create_gui)
    create_gui_thread.start()
    main_loop()