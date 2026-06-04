import time
import threading
import serial
import serial.tools.list_ports
import pystray
from PIL import Image, ImageDraw
import ctypes
from ctypes import wintypes
import logging
import os
import sys

# Thư viện cho Audio Visualizer
try:
    from pycaw.pycaw import AudioUtilities, IAudioMeterInformation
    from comtypes import CLSCTX_ALL
    import comtypes
    audio_libraries_available = True
except ImportError:
    audio_libraries_available = False

log_path = os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), 'bongo_debug.log')
logging.basicConfig(filename=log_path, level=logging.INFO, 
                    format='%(asctime)s - %(levelname)s - %(message)s')
logging.info("BongoCatClient v15 Beat Visualizer Started")

ser = None
connected = False
toggle_hand = False
running = True
key_press_count = 0        # Đếm số phím
key_press_lock = threading.Lock()  # Thread-safe lock

def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    
    # Priority 1: Specific USB-to-UART bridge chips
    for port, desc, hwid in sorted(ports):
        if "CH340" in desc or "CP210" in desc:
            return port
            
    # Priority 2: ESP32-C3 Native USB CDC (usually "USB Serial Device" or "USB JTAG")
    for port, desc, hwid in sorted(ports):
        # Ignore Intel AMT and Standard Serial over Bluetooth
        if "AMT" in desc or "Standard" in desc or "Bluetooth" in desc or "Communications Port" in desc:
            continue
        if "USB Serial Device" in desc or "JTAG" in desc:
            return port
            
    # Priority 3: Any USB serial port (last resort)
    for port, desc, hwid in sorted(ports):
        if "AMT" in desc or "Standard" in desc or "Bluetooth" in desc or "Communications Port" in desc:
            continue
        if "USB" in desc or "Serial" in desc:
            return port
            
    return None

def connect_serial():
    global ser, connected, running
    while running:
        if not connected:
            port = find_esp32_port()
            if port:
                try:
                    ser = serial.Serial()
                    ser.port = port
                    ser.baudrate = 115200
                    ser.timeout = 0.1
                    ser.setDTR(False)
                    ser.setRTS(False)
                    ser.open()
                    connected = True
                    logging.info(f"Successfully connected to {port}")
                except Exception as e:
                    logging.error(f"Failed to connect to {port}: {e}")
        time.sleep(3)

def serial_worker():
    global key_press_count, ser, connected, toggle_hand, running
    while running:
        count = 0
        with key_press_lock:
            count = key_press_count
            key_press_count = 0
        
        if count > 0 and connected and ser and ser.is_open:
            try:
                for _ in range(count):
                    char_to_send = b'K:L\n' if toggle_hand else b'K:R\n'
                    toggle_hand = not toggle_hand
                    ser.write(char_to_send)
            except Exception as e:
                logging.error(f"Serial write error: {e}")
                connected = False
        time.sleep(0.005)

# --- AUDIO VISUALIZER WORKER ---
def init_audio_meter():
    if not audio_libraries_available:
        return None
    try:
        from pycaw.pycaw import IMMDeviceEnumerator
        from pycaw.pycaw import IAudioMeterInformation
        import comtypes
        
        CLSID_MMDeviceEnumerator = comtypes.GUID("{BCDE0395-E52F-467C-8E3D-C4579291692E}")
        eRender = 0
        eMultimedia = 1
        
        deviceEnumerator = comtypes.CoCreateInstance(
            CLSID_MMDeviceEnumerator,
            IMMDeviceEnumerator,
            comtypes.CLSCTX_INPROC_SERVER)
        speakers = deviceEnumerator.GetDefaultAudioEndpoint(eRender, eMultimedia)
        
        interface = speakers.Activate(IAudioMeterInformation._iid_, CLSCTX_ALL, None)
        meter = ctypes.cast(interface, ctypes.POINTER(IAudioMeterInformation))
        return meter
    except Exception as e:
        logging.error(f"init_audio_meter error: {e}")
        return None

def audio_worker():
    global ser, connected, running
    try:
        logging.info("Audio worker thread started")
        if not audio_libraries_available:
            logging.warning("pycaw or comtypes not installed. Beat visualizer is disabled.")
            return
            
        comtypes.CoInitialize()
        logging.info("Audio worker initialized COM.")
        last_val = -1
        
        meter = init_audio_meter()
        
        while running:
            if connected and ser and ser.is_open:
                val_int = 0
                if meter:
                    try:
                        val_float = meter.GetPeakValue()
                        val_int = int(val_float * 255)
                    except Exception as e:
                        # If device disconnected or invalidated, try to reinitialize
                        logging.error(f"GetPeakValue error: {e}. Reinitializing...")
                        meter = init_audio_meter()
                
                if val_int != last_val:
                    try:
                        ser.write(f"A:{val_int}\n".encode())
                        last_val = val_int
                    except Exception as e:
                        logging.error(f"Serial audio write error: {e}")
                        connected = False
            time.sleep(0.04) # ~25 FPS
            
        try:
            comtypes.CoUninitialize()
        except Exception:
            pass
        logging.info("Audio worker thread cleanly exited")
    except BaseException as e:
        logging.error(f"FATAL error in audio_worker: {e}", exc_info=True)
        with open("fatal_audio_error.txt", "w") as f:
            f.write(str(e))

# --- LOW LEVEL KEYBOARD HOOK ---
user32 = ctypes.WinDLL('user32', use_last_error=True)
WH_KEYBOARD_LL = 13
WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
WM_SYSKEYDOWN = 0x0104
WM_SYSKEYUP = 0x0105

class KBDLLHOOKSTRUCT(ctypes.Structure):
    _fields_ = [
        ("vkCode", wintypes.DWORD),
        ("scanCode", wintypes.DWORD),
        ("flags", wintypes.DWORD),
        ("time", wintypes.DWORD),
        ("dwExtraInfo", ctypes.c_void_p)
    ]

LRESULT = ctypes.c_ssize_t
HOOKPROC = ctypes.WINFUNCTYPE(LRESULT, ctypes.c_int, wintypes.WPARAM, ctypes.POINTER(KBDLLHOOKSTRUCT))

user32.CallNextHookEx.argtypes = [ctypes.c_void_p, ctypes.c_int, wintypes.WPARAM, ctypes.c_void_p]
user32.CallNextHookEx.restype = LRESULT
user32.SetWindowsHookExW.argtypes = [ctypes.c_int, HOOKPROC, wintypes.HINSTANCE, wintypes.DWORD]
user32.SetWindowsHookExW.restype = ctypes.c_void_p
user32.UnhookWindowsHookEx.argtypes = [ctypes.c_void_p]
user32.UnhookWindowsHookEx.restype = wintypes.BOOL

hook_id = None
key_states = [False] * 256
LLKHF_INJECTED = 0x10

def hook_callback(nCode, wParam, lParam):
    global key_press_count
    if nCode >= 0 and lParam:
        kbd = lParam.contents
        vk = kbd.vkCode
        if (kbd.flags & LLKHF_INJECTED) == 0:
            if vk < 256:
                if wParam == WM_KEYDOWN or wParam == WM_SYSKEYDOWN:
                    if not key_states[vk]:
                        key_states[vk] = True
                        with key_press_lock:
                            key_press_count += 1
                elif wParam == WM_KEYUP or wParam == WM_SYSKEYUP:
                    key_states[vk] = False
    return user32.CallNextHookEx(hook_id, nCode, wParam, ctypes.cast(lParam, ctypes.c_void_p))

hook_func = HOOKPROC(hook_callback)

def hook_thread_func():
    global hook_id, running
    hook_id = user32.SetWindowsHookExW(WH_KEYBOARD_LL, hook_func, 0, 0)
    if not hook_id:
        logging.error("Failed to install keyboard hook")
        return
    msg = wintypes.MSG()
    while running:
        if user32.PeekMessageW(ctypes.byref(msg), 0, 0, 0, 1):
            if msg.message == 0x0012: # WM_QUIT
                break
            user32.TranslateMessage(ctypes.byref(msg))
            user32.DispatchMessageW(ctypes.byref(msg))
        else:
            time.sleep(0.01)
    user32.UnhookWindowsHookEx(hook_id)

def create_image():
    image = Image.new('RGB', (64, 64), color = (255, 255, 255))
    draw = ImageDraw.Draw(image)
    draw.ellipse((16, 16, 48, 48), fill=(0, 0, 0))
    return image

def setup_tray(icon):
    icon.visible = True

def quit_app(icon, item):
    global running
    running = False
    icon.stop()

if __name__ == '__main__':
    mutex_name = "Global\\BongoCatClient_v15_Unique_Mutex"
    mutex = ctypes.windll.kernel32.CreateMutexW(None, False, mutex_name)
    last_error = ctypes.windll.kernel32.GetLastError()
    if last_error == 183: # ERROR_ALREADY_EXISTS
        sys.exit(0)
        
    threading.Thread(target=connect_serial, daemon=True).start()
    threading.Thread(target=serial_worker, daemon=True).start()
    threading.Thread(target=audio_worker, daemon=True).start()
    threading.Thread(target=hook_thread_func, daemon=True).start()

    icon = pystray.Icon("BongoCat", create_image(), "Bongo Cat Clock", menu=pystray.Menu(
        pystray.MenuItem("Quit", quit_app)
    ))
    try:
        icon.run(setup_tray)
    except Exception as e:
        logging.error(f"System tray error: {e}")