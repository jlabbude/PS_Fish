import cv2
import datetime
import numpy as np
import time
import pygetwindow as gw
from pynput.mouse import Controller, Button
import pyautogui
import subprocess
import math

def should_click(x1, y1, width1, height1):
    cmd = ["./compareScreenshots", 
           str(x1), str(y1), 
           str(width1), str(height1)
        ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.stdout.strip() == "true"
    
def find_window_by_application_name(app_name):
    windows = gw.getAllWindows()
    for window in windows:
        if window.title.startswith(app_name):
            return window
    return None

def find_thing_onscreen(array_index, confidence):

    image_array=('fishing_button.png', 'fishing_bar.png')

    try:
        button_location = pyautogui.locateOnScreen(image_array[array_index], confidence=confidence)
        return button_location
    except pyautogui.ImageNotFoundException:
        return None

def main():
    desired_window = find_window_by_application_name("Roblox")
    if desired_window:

        desired_window.activate()
        print("Window found")

        fishing_stage=0

        last_click_time = time.time()

        while fishing_stage < 2:
            if find_thing_onscreen(0, 0.85):
                print(f"Fishing started:", fishing_stage)
                if fishing_stage == 0:
                    pyautogui.moveTo(find_thing_onscreen(0, 0.85).left + 
                                     find_thing_onscreen(0, 0.85).width / 2,
                                     find_thing_onscreen(0, 0.85).top + 
                                     find_thing_onscreen(0, 0.85).height / 2)
                    time.sleep(0.5)
                if fishing_stage == 1:

                    #get region
                    fishing_bar_region = find_thing_onscreen(1, 0.85)

                    while find_thing_onscreen(0, 0.85) != None:
                        if should_click():
                            current_time = time.time()
                            if current_time - last_click_time > 1:  # Adjust the time interval as needed
                                pyautogui.click()
                                last_click_time = current_time
                        else:
                            print("Not")
                            print(datetime.datetime.now().strftime("%H:%M:%S.%f"))
                fishing_stage += 1
            else:
                print("Not fishing")
            time.sleep(1)
	
    else:
        print("Window not found")


main()