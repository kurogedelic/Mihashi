#!/usr/bin/env python3
"""
Mihashi Automatic Firmware Upload Script
Automatically triggers bootloader mode and uploads firmware
"""

import serial
import serial.tools.list_ports
import time
import shutil
import glob
import os
import sys

# Device configurations
DEVICES = {
    'mihashi': {
        'vid_pid': ['2E8A:0005', '2E8A:000A'],  # RP2350 USB Serial + Bootloader
        'bootloader_baud': 3000,  # RP2350 might use 3000bps
        'firmware_pattern': '**/mihashi*.uf2',
        'mount_name': 'RP2350'
    },
    'littlejoe': {
        'vid_pid': ['2341:804D', '2341:004D'],  # XIAO SAMD21 Serial + Bootloader  
        'bootloader_baud': 1200,  # SAMD21 uses 1200bps
        'firmware_pattern': '**/LittleJoe*.ino.bin',
        'mount_name': 'Arduino'
    },
    'uartmonitor': {
        'vid_pid': ['2341:804D', '2341:004D'],  # XIAO SAMD21 Serial + Bootloader
        'bootloader_baud': 1200,  # SAMD21 uses 1200bps  
        'firmware_pattern': '**/UartMonitor*.ino.bin',
        'mount_name': 'Arduino'
    }
}

def find_device_port(device_name):
    """Find USB Serial port for device"""
    config = DEVICES[device_name]
    ports = serial.tools.list_ports.comports()
    
    print(f"Searching for {device_name}...")
    for port in ports:
        if port.vid is not None and port.pid is not None:
            vid_pid = f"{port.vid:04X}:{port.pid:04X}".upper()
            print(f"Found port: {port.device} ({vid_pid}) - {port.description}")
            if vid_pid in config['vid_pid']:
                print(f"Found {device_name} at {port.device} ({vid_pid})")
                return port.device
        else:
            print(f"Found port: {port.device} (VID:PID unknown) - {port.description}")
    
    print(f"Device {device_name} not found")
    print("Available VID:PID combinations:", [f"{p.vid:04X}:{p.pid:04X}" if p.vid and p.pid else "Unknown" for p in ports])
    return None

def trigger_bootloader(port, baud_rate):
    """Trigger bootloader mode by opening serial at specific baud rate"""
    try:
        print(f"Triggering bootloader on {port} at {baud_rate}bps...")
        ser = serial.Serial(port, baud_rate)
        time.sleep(0.1)  # Brief connection
        ser.close()
        print("Bootloader trigger sent")
        time.sleep(2)  # Wait for bootloader mode
        return True
    except Exception as e:
        print(f"Failed to trigger bootloader: {e}")
        return False

def wait_for_mount(mount_name, timeout=10):
    """Wait for bootloader mount point to appear"""
    print(f"Waiting for {mount_name} mount...")
    
    for i in range(timeout):
        # Check for mounted drives
        if sys.platform == "darwin":  # macOS
            mount_path = f"/Volumes/{mount_name}"
            if os.path.exists(mount_path):
                print(f"Found mount at {mount_path}")
                return mount_path
        elif sys.platform == "linux":  # Linux
            mount_paths = [f"/media/{mount_name}", f"/mnt/{mount_name}"]
            for path in mount_paths:
                if os.path.exists(path):
                    print(f"Found mount at {path}")
                    return path
        
        time.sleep(1)
    
    print(f"Mount {mount_name} not found after {timeout}s")
    return None

def find_firmware(pattern):
    """Find firmware file matching pattern"""
    files = glob.glob(pattern, recursive=True)
    if files:
        # Get most recent file
        latest = max(files, key=os.path.getmtime)
        print(f"Found firmware: {latest}")
        return latest
    
    print(f"No firmware found matching: {pattern}")
    return None

def upload_firmware(device_name, firmware_path=None):
    """Complete firmware upload process"""
    config = DEVICES[device_name]
    
    # Find device port
    port = find_device_port(device_name)
    if not port:
        return False
    
    # Trigger bootloader
    if not trigger_bootloader(port, config['bootloader_baud']):
        return False
    
    # Wait for mount
    mount_path = wait_for_mount(config['mount_name'])
    if not mount_path:
        return False
    
    # Find firmware if not specified
    if not firmware_path:
        firmware_path = find_firmware(config['firmware_pattern'])
        if not firmware_path:
            return False
    
    # Copy firmware
    try:
        dest_path = os.path.join(mount_path, os.path.basename(firmware_path))
        print(f"Copying {firmware_path} to {dest_path}")
        shutil.copy2(firmware_path, dest_path)
        print(f"Upload complete: {device_name}")
        return True
    except Exception as e:
        print(f"Upload failed: {e}")
        return False

def main():
    """Main upload script"""
    if len(sys.argv) < 2:
        print("Usage: upload_firmware.py <device_name> [firmware_path]")
        print("Devices:", ", ".join(DEVICES.keys()))
        return
    
    device_name = sys.argv[1]
    firmware_path = sys.argv[2] if len(sys.argv) > 2 else None
    
    if device_name not in DEVICES:
        print(f"Unknown device: {device_name}")
        print("Available devices:", ", ".join(DEVICES.keys()))
        return
    
    success = upload_firmware(device_name, firmware_path)
    if success:
        print(f"✅ {device_name} firmware uploaded successfully")
    else:
        print(f"❌ {device_name} firmware upload failed")

if __name__ == "__main__":
    main()