import socket
import cv2
import time
import struct
import sys
import numpy as np

# ------------------------------
# Configuration
# ------------------------------
SERVER_IP = "led-matrix.lan"
SERVER_PORT = 1234

LOOP_VIDEO = True  # Set to True to loop; only makes sense for video files

# Display size from your ESP32 code (e.g., 128×128 for SSD1327)
WIDTH = 192
HEIGHT = 32

# Frame rate limit (in seconds per frame). 
# 0.03 ≈ 33 ms per frame => ~30 FPS.
FRAME_DELAY = 0.03


def main():
    # Check command-line arguments
    if len(sys.argv) < 2:
        print("Usage: python send-video.py <path_to_video>")
        sys.exit(1)

    VIDEO_SOURCE = sys.argv[1]  # The first argument after the script name
    
    # Try opening the VIDEO
    try:
        cap = cv2.VideoCapture(VIDEO_SOURCE)
    except Exception as e:
        print(f"Error opening Video file '{VIDEO_SOURCE}': {e}")
        return

    
    # Open the video source
    cap = cv2.VideoCapture(VIDEO_SOURCE)
    if not cap.isOpened():
        print("ERROR: Unable to open video source:", VIDEO_SOURCE)
        return

    # Create the TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print(f"Connecting to {SERVER_IP}:{SERVER_PORT}...")
        s.connect((SERVER_IP, SERVER_PORT))
        print("Connected!")

        try:
            while True:
                # Read one frame
                ret, frame = cap.read()

                # If we didn't get a frame, we've likely reached the end of a file
                if not ret:
                    if LOOP_VIDEO:
                        # Reset video position to the first frame
                        cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                        continue  # Attempt to read again
                    else:
                        print("No more frames. Exiting...")
                        break

                # Convert to 8-bit grayscale (single channel)
                gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

                # Resize to match display size
                resized_gray = cv2.resize(gray_frame, (WIDTH, HEIGHT), interpolation=cv2.INTER_AREA)

                # Convert to bytes
                frame_bytes = resized_gray.tobytes()

                frame_size  = len(frame_bytes)  # Should be WIDTH*HEIGHT

                # 1) Send 4-byte header (big-endian) with the frame size
                # struct.pack('>I', frame_size) means: 
                # ">" = big-endian, "I" = 4-byte unsigned int
                header = struct.pack('>I', frame_size)
                s.sendall(header)

                # Send the frame over TCP
                s.sendall(frame_bytes)

                # Optional delay to control frame rate
                time.sleep(FRAME_DELAY)

        except KeyboardInterrupt:
            print("\nInterrupted by user.")
        finally:
            cap.release()
            print("Video capture released. Socket closed.")


if __name__ == "__main__":
    main()