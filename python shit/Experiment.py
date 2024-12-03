import pygame
import sys
import random
import psutil
import time
import threading
import queue

# Initialize Pygame
pygame.init()

GAMEFPS = 60

# Screen setup
WIDTH, HEIGHT = 1920, 1080
screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.FULLSCREEN)
pygame.display.set_caption("2.5D Plane")

# Colors
BACKGROUND_COLOR = (135, 206, 235)
WATER_COLOR = (0, 0, 255)  # Blue for water
GRASS_COLOR = (34, 139, 34)  # Land color
GRID_COLOR = (10,10,10)
DEBUG_BACKGROUND_COLOR = (0, 0, 0, 150)  # Semi-transparent background for the debug menu
DEBUG_TEXT_COLOR = (255, 255, 255)  # White text color

# Grid setup
GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH  = 20, 20, 10  # Grid size
BASE_TILE_SIZE = 60  # Base tile size (before zoom)
zoom = 1.0  # Initial zoom level
MIN_ZOOM, MAX_ZOOM = 0.5, 2.0  # Zoom limits

# Lake setup
lake_size = 3  # 3x3 lakes

# Camera setup
camera_x, camera_y = 0, 0
dragging = False
last_mouse_pos = (0, 0)


# Font for displaying debug information
font = pygame.font.SysFont("Arial", 18)

# Queue for passing debug info between threads
debug_info_queue = queue.Queue()

# pid for this process
pid = psutil.Process()

# Function to convert grid (x, y) to screen (isometric) coordinates
class Renderer():
    def __init__(self):
        for x in range(GRID_WIDTH):             # Width
            for y in range(GRID_HEIGHT):        # Depth
                tile_size = int(BASE_TILE_SIZE * zoom)
                self.iso_center = [(x - y) * tile_size // 2 + WIDTH // 2 + camera_x, (x + y) * tile_size // 4 + HEIGHT // 8 + camera_y]
                self.iso_x = self.iso_center[0]
                self.iso_y = self.iso_center[1]
                # Otherwise, draw land
                points=[
                    (self.iso_x, self.iso_y),
                    (self.iso_x + tile_size // 2, self.iso_y + tile_size // 4),
                    (self.iso_x, self.iso_y + tile_size // 2),
                    (self.iso_x - tile_size // 2, self.iso_y + tile_size // 4)
                ]
                pygame.draw.polygon(screen, GRASS_COLOR, points)   # squares
                pygame.draw.polygon(screen, GRID_COLOR, points, 2) # tile border

    # Function to draw the grid and lakes
    def draw_grid(self):
        for x in range(GRID_WIDTH):             # Width
            for y in range(GRID_HEIGHT):        # Depth
                self.iso_x += self.iso_center[0] + camera_x
                self.iso_y += self.iso_center[1] + camera_y
                
                # Adjust tile size based on zoom
                tile_size = int(BASE_TILE_SIZE * zoom)
                
                # Draw each tile
                
                # Otherwise, draw land
                points = [
                    (self.iso_x, self.iso_y),
                    (self.iso_x + tile_size // 2, self.iso_y + tile_size // 4),
                    (self.iso_x, self.iso_y + tile_size // 2),
                    (self.iso_x - tile_size // 2, self.iso_y + tile_size // 4)
                ]
                pygame.draw.polygon(screen, GRASS_COLOR, points)
                pygame.draw.polygon(screen, GRID_COLOR, points, 2)  # Tile border


# Function to calculate FPS and CPU usage in a separate thread
def calculate_debug_info():
    frame_times = []
    prev_time = pygame.time.get_ticks()
    cpu_usage_list = []  # List to store CPU usage readings
    avg_cpu_usage = 0

    while True:
        current_time = pygame.time.get_ticks()
        frame_time = current_time - prev_time  # Calculate frame time (tick period)
        prev_time = current_time
        
        # Calculate FPS (average over the last 60 frames)
        if frame_time > 0:
            # Calculate FPS (average over the last 60 frames)
            frame_times.append(frame_time)
            if len(frame_times) > 60:
                frame_times.pop(0)
            avg_frame_time = sum(frame_times) / len(frame_times)
            fps = 1000 / avg_frame_time  # FPS is 1000ms / avg frame time in ms
        else:
            fps = 60  # Default FPS if frame_time is zero or too small
        
         # Get CPU usage using psutil
        cpu_usage_list.append(pid.cpu_percent()*0.1)

        # Every 10 frames, calculate the average CPU usage and send it to the main thread
        if len(cpu_usage_list) >= 15:
            avg_cpu_usage = sum(cpu_usage_list) / 15  # Average CPU usage over 10 frames
            cpu_usage_list = []  # Reset the list after averaging
            debug_info_queue.put((fps, frame_time, avg_cpu_usage))  # Put averaged data into the queue
        else:
            # Send current FPS and tick period if not yet 10 frames
            last_cpu_usage = avg_cpu_usage
            debug_info_queue.put((fps, frame_time, last_cpu_usage))  # No CPU usage data yet

        time.sleep(1/GAMEFPS)
        
# Main game loop
def main():
    global camera_x, camera_y, dragging, last_mouse_pos, zoom
    
    # Generate random lakes without overlap checking
    
    clock = pygame.time.Clock()

    debug_menu_visible = False

    frame_counter = 0 

    # Start the debug calculation thread
    debug_thread = threading.Thread(target=calculate_debug_info, daemon=True)
    debug_thread.start()
    
    debug_menu_visible = False
    renderer = Renderer()
    while True:
        screen.fill(BACKGROUND_COLOR)
        renderer.draw_grid()
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
                
            # Check for middle mouse button events
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 2:  # Middle mouse button
                    dragging = True
                    last_mouse_pos = pygame.mouse.get_pos()
                    
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 2:
                    dragging = False
                    
            # Handle mouse wheel for zooming
            elif event.type == pygame.MOUSEWHEEL:
                # Get the mouse position before zooming
                mouse_x, mouse_y = pygame.mouse.get_pos()

                # Calculate the zoom difference
                zoom_change = event.y * 0.1  # Zoom speed
                new_zoom = zoom + zoom_change
                new_zoom = max(MIN_ZOOM, min(MAX_ZOOM, new_zoom))  # Clamp zoom level

                # Calculate zoom difference
                zoom_factor = new_zoom / zoom
                zoom = new_zoom  # Update the zoom level

                # Adjust camera position to keep the mouse position centered
                camera_x -= (mouse_x - WIDTH // 2) * (zoom_factor - 1)
                camera_y -= (mouse_y - HEIGHT // 2) * (zoom_factor - 1)

            # Toggle debug menu on F3 key press
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_F3:
                    debug_menu_visible = not debug_menu_visible
                elif event.key == pygame.K_ESCAPE:
                    pygame.quit()
                    sys.exit()

        # Handle dragging movement
        if dragging:
            current_mouse_pos = pygame.mouse.get_pos()
            dx = current_mouse_pos[0] - last_mouse_pos[0]
            dy = current_mouse_pos[1] - last_mouse_pos[1]
            camera_x += dx
            camera_y += dy
            last_mouse_pos = current_mouse_pos  # Update last position

# Get the latest debug info from the background thread
        if not debug_info_queue.empty():
            fps, frame_time, cpu_usage = debug_info_queue.get()

            # Draw the debug menu if visible
            if debug_menu_visible:
                # Draw a semi-transparent background for the debug menu
                pygame.draw.rect(screen, DEBUG_BACKGROUND_COLOR, (10, 10, 250, 120))
                
                # Render FPS, Tick Period text
                fps_text = font.render(f"FPS: {fps:.2f}", True, DEBUG_TEXT_COLOR)
                tick_period_text = font.render(f"Tick Period: {frame_time:.2f} ms", True, DEBUG_TEXT_COLOR)
                
                # Render CPU usage
                

                cpu_text = font.render(f"CPU Usage: {cpu_usage:.3f}%", True, DEBUG_TEXT_COLOR)


                # Blit the text on the screen
                screen.blit(fps_text, (20, 20))
                screen.blit(tick_period_text, (20, 40))
                screen.blit(cpu_text, (20, 60))
        
        pygame.display.flip()

        # Limit FPS to 60
        clock.tick(GAMEFPS)
        
if __name__ == "__main__":
    main()