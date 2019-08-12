#!/usr/bin/env python

import sys
import time
import pygame
import serial

pygame.init()                                                 #initiaise pygame
controls = pygame.joystick.Joystick(0)    # call the joystic controls
clock = pygame.time.Clock()                     # intialise pygame refresh and call it clock
controls.init()                                                # initialise the controls

#arduino =  serial.Serial('/dev/ttyUSB0', 9600,timeout = 1) #connect to the arduino's serial port
#time.sleep(2)

arduino =  serial.Serial('COM9', 9600,timeout = 1) #connect to the arduino's serial port
time.sleep(2)

EXIT = False

old_min = -1
old_max = 1
new_min = 100
new_max = 355

def valueMap(old_value):
    new_value = str(int(round(( ( old_value - old_min ) / (old_max - old_min) ) * (new_max - new_min) + new_min)))
    return(new_value)
    
while not EXIT:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            EXIT = True
    
    controllerName = str(controls.get_name())
    axesNumber = controls.get_numaxes()
    hatNumber = controls.get_numhats()
    buttonNumber = controls.get_numbuttons()

    a = valueMap(controls.get_axis(0))   
    b = valueMap(controls.get_axis(1))  
    c = valueMap(controls.get_axis(2))
    d = valueMap(controls.get_axis(3))
    e = '000'
    f = '0000'
    
    for x in range(0,buttonNumber):
        if controls.get_button(x) == 1:
            e = '0' + str(x+1)
            if x < 9:
                e = '00' + str(x+1)
                
    for positionHat in range(hatNumber):
        hat = controls.get_hat(positionHat)
        if hat[0] == -1:
            f = '1000'
        elif hat[0] == 1:
            f = '0100'
        if hat[1] == 1:
            f = '0010'
        elif hat[1] == -1:
            f = '0001'
            
    control = ['<' + a,b,c,d,e,f + '>']      # save strings in a list
    cstring = ",".join(control)                # convert list to a single string with commas seperating values
                
    print(cstring)
    arduino.write(cstring)                      # print string to shell and write data to arduino with a 0.1ms delay
    time.sleep(0.0001)
    
    if  controls.get_button(12) == 1:
        pygame.quit()                               # This is used to quit pygame and use any internal program within the python
        quit()

    clock.tick(1000)

pygame.quit()
quit()
