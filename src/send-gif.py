import socket
import time
import struct
import sys
from PIL import Image

# -----------------------------------------------------------
# Configuration
# -----------------------------------------------------------
SERVER_IP   = "led-matrix.lan"  # ESP32 IP address
SERVER_PORT = 1234             # Same port as ESP32 code
LOOP_GIF    = True

# The display dimensions from your ESP32 code (e.g., 128×128)
WIDTH  = 192
HEIGHT = 32

def main():
    # Check command-line arguments
    if len(sys.argv) < 2:
        print("Usage: python send-gif.py <path_to_gif>")
        sys.exit(1)

    gif_file = sys.argv[1]  # The first argument after the script name
    
    # Try opening the GIF
    try:
        gif = Image.open(gif_file)
    except Exception as e:
        print(f"Error opening GIF file '{gif_file}': {e}")
        return


    # Create the TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print(f"Connecting to {SERVER_IP}:{SERVER_PORT}...")
        s.connect((SERVER_IP, SERVER_PORT))
        print("Connected!")

        try:
            while True:  # Loop the GIF indefinitely if LOOP_GIF is True
                for frame_index in range(gif.n_frames):
                    gif.seek(frame_index)

                    # Convert to 8-bit grayscale
                    frame = gif.convert("L")
                    # Resize
                    frame = frame.resize((WIDTH, HEIGHT))

                    # Convert image to raw bytes
                    frame_bytes = frame.tobytes()
                    frame_size  = len(frame_bytes)  # Should be WIDTH*HEIGHT

                    # 1) Send 4-byte header (big-endian) with the frame size
                    # struct.pack('>I', frame_size) means: 
                    # ">" = big-endian, "I" = 4-byte unsigned int
                    header = struct.pack('>I', frame_size)
                    s.sendall(header)

                    # 2) Send the actual frame data
                    s.sendall(frame_bytes)

                    # Use GIF's frame duration (or default 100ms)
                    duration_ms = gif.info.get("duration", 100)
                    time.sleep(duration_ms / 1000.0)

                if not LOOP_GIF:
                    break

        except KeyboardInterrupt:
            print("\nInterrupted by user.")
        finally:
            gif.close()
            print("GIF closed. Socket closed.")


if __name__ == "__main__":
    main()
