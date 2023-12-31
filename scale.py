import pygame

# Initialize pygame
pygame.init()

# Framebuffer size
framebuffer_size = (240, 240)

# Window size (scaled)
window_size = (480, 480)

# Set up the window
screen = pygame.display.set_mode(window_size)
pygame.display.set_caption("Scaled Framebuffer")

# Create a surface for the framebuffer
framebuffer = pygame.Surface(framebuffer_size)

# Main loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Draw on the framebuffer
    framebuffer.fill((0, 0, 0))  # Clear with black
    pygame.draw.circle(framebuffer, (255, 0, 0), (120, 120), 50)  # Draw a red circle

    # Scale and display the framebuffer
    scaled_framebuffer = pygame.transform.scale(framebuffer, window_size)
    screen.blit(scaled_framebuffer, (0, 0))
    pygame.display.flip()

# Quit pygame
pygame.quit()