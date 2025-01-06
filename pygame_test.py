import pygame
import numpy as np
import asyncio
import sys
import struct
import numpy as np
from datetime import datetime

# LED matrix resolution
WIDTH, HEIGHT = 192, 32

SUPER_RES_SCALE = 1
SUPER_WIDTH = WIDTH * SUPER_RES_SCALE
SUPER_HEIGHT = HEIGHT * SUPER_RES_SCALE

# TCP server info (LED controller side)
HOST = "led-matrix.lan"
PORT = 1234

# Frames per second
FPS = 30

async def open_connection():
    """
    Opens a persistent TCP connection to the LED controller.
    Returns (reader, writer) for async I/O.
    """
    reader, writer = await asyncio.open_connection(HOST, PORT)
    print(f"[INFO] Connected to {HOST}:{PORT}")
    return reader, writer


async def send_frames(queue, writer):
    """
    Async consumer that retrieves frames from the queue and sends them over the network.
    """
    while True:
        # Wait for a frame from the queue
        frame = await queue.get()

        if frame is None:
            # Signal to exit the loop (None is a sentinel value)
            break

        # Send the frame over the persistent TCP connection

        """
        Convert the (WIDTH x HEIGHT) grayscale NumPy array into bytes and send it.
        """
        # Ensure 8-bit (0..255)
        pixel_data_8bit = frame.astype(np.uint8)

        # Transpose to row-major order if needed
        pixel_data_row_major = pixel_data_8bit.T

        # Flatten (width * height) -> bytes
        frame_bytes = pixel_data_row_major.flatten().tobytes()

        frame_size  = len(frame_bytes)  # Should be WIDTH*HEIGHT

        # 1) Send 4-byte header (big-endian) with the frame size
        # struct.pack('>I', frame_size) means: 
        # ">" = big-endian, "I" = 4-byte unsigned int
        header = struct.pack('>I', frame_size)
        writer.write(header)
        writer.write(frame_bytes)
        await writer.drain()

        queue.task_done()

async def run_pygame(queue):
    """
    Main loop for Pygame + async sending of frames.
    - Opens a persistent connection
    - Runs the Pygame loop
    - Sends each frame asynchronously
    - Closes the connection on exit
    """
    # 1) Open the persistent connection
    # reader, writer = await open_connection()

    print("Run game")
    
    # 2) Initialize Pygame
    pygame.init()

    screen = pygame.display.set_mode((WIDTH, HEIGHT))

    super_surface = pygame.Surface((SUPER_WIDTH, SUPER_HEIGHT))
    
    pygame.display.set_caption("Pygame LED Matrix Example (Async TCP)")

    clock = pygame.time.Clock()

    # Example: animate a white rectangle
    rect_x = 0
    rect_y = 8
    rect_width = 32
    rect_height = 32
    speed = 1

    font = pygame.font.Font(None, 48)  # Adjust size for super-resolution

    running = True
    while running:
        # --- Pygame event loop ---
        for event in pygame.event.get():
            # print("EVENT:", event.type, event)
            if event.type == pygame.QUIT:
                running = False


        current_time = datetime.now().strftime("%H:%M:%S")

        # --- Update animation ---
        rect_x += speed
        if rect_x > SUPER_WIDTH:
            rect_x = -rect_width

        # --- Draw ---
        super_surface.fill((0, 0, 0))
        
        time_surface = font.render(current_time, True, (128, 128, 128))
        super_surface.blit(time_surface, (50, 10))  # Position at (50, 10)

        pygame.draw.rect(super_surface, (32, 0, 0),
                         (rect_x, rect_y, rect_width, rect_height))

        pygame.display.flip()

        scaled_surface = pygame.transform.smoothscale(super_surface, (WIDTH, HEIGHT))

        screen.blit(scaled_surface, (0, 0))

        # --- Extract frame (monochrome) ---
        pixel_data_3d = pygame.surfarray.array3d(scaled_surface)  # shape: (width, height, 3)
        pixel_data_gray = pixel_data_3d[..., 0]           # just take red channel for monochrome


        # --- Send the frame over TCP ---
        # Note: This is an async call, so we 'await' it.
        if queue.qsize() < 10:  # Optional: Drop frames if the queue is too large
            await queue.put(pixel_data_gray)

        # --- Control frame rate ---
        # clock.tick() is blocking, which somewhat defeats the purpose of async,
        # but for simplicity, we'll keep it. You can remove it and use
        # `await asyncio.sleep(1 / FPS)` if you prefer purely async timing.
        # clock.tick(FPS)
        await asyncio.sleep(1 / FPS)

    # --- Cleanup ---
    await queue.put(None)
    pygame.quit()
    print("[INFO] Pygame closed")

async def main():
    """
    Entry point: runs the asyncio event loop for our Pygame routine.
    """

    queue = asyncio.Queue()
    reader, writer = await open_connection()

    try:
        await asyncio.gather(
            run_pygame(queue),
            send_frames(queue, writer)
        )
    except KeyboardInterrupt:
        print("interrupt")
        pass
    except Exception as e:
        print(f"[ERROR] {e}")
        raise
    finally:
        writer.close()
        await writer.wait_closed()
        print("[INFO] Connection closed.")
        sys.exit()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("[INFO] KeyboardInterrupt detected.")
        sys.exit()
