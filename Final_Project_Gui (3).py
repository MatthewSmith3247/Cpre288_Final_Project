from tkinter import *
import tkinter

#USE PIP INSTALL in the Terminal line

#Had to pip install SpeechRecognition and pyaudio for listener
import speech_recognition as sr 

import threading
#for speaker install pyttsx3
#import pyttsx3

#For Socket Connection
import time
import socket
# See: Background, Socket API Overview, and TCP Sockets    
import re

#Import/Include useful math and plotting functions
import numpy as np
import matplotlib.pyplot as plt
import os  # import function for finding absolute path to this python script

#pip install matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as patches

import numpy as np

#---------------------------------GUI Set-up
#create window named root
root = Tk()

#set a constant window size, that is still resizable
root.geometry("800x800")
root.resizable(width=True, height=True)

#define Image
bg = PhotoImage(file="robot.png")

#set image
my_canvas = Canvas(root, width=800, height=800)
my_canvas.pack(fill="both", expand=True)
my_canvas.create_image(0,0,image=bg, anchor="nw")
#---------------------------------END GUI SET UP

#---------------------------------SOCKET SET UP
# TCP Socket BEGIN (See Echo Client example): https://realpython.com/python-sockets/#echo-client-and-server
HOST = "192.168.1.1"  # The server's hostname or IP address
PORT = 288       # The port used by the server
cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
cybot_socket.connect((HOST, PORT))   # Connect to the socket  (Note: Server must first be running)
                      
cybot = cybot_socket.makefile("rbw", buffering=0)  # makefile creates a file object out of a socket:  https://pythontic.com/modules/socket/makefile
# TCP Socket END

# Send some text: Either 1) Choose "Hello" or 2) have the user enter text to send
send_message = "Hello\n"                            # 1) Hard code message to "Hello", or
# send_message = input("Enter a message:") + '\n'   # 2) Have user enter text

cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

print("Sent to server: " + send_message) 
print("wait for server reply\n")
rx_message = cybot.readline()      # Wait for a message, readline expects message to end with "\n"
print("Got a message from server: " + rx_message.decode() + "\n") # Convert message from bytes to String (i.e., decode)
#---------------------------------END SOCKET SET UP

#---------------------------------BUTTON  SET UP
#variables for later
global word_said, text_to_speak, jokeCount, manualMode, buttonFunVal,flag_updateGraph, count
count = 0
flag_updateGraph = 0
buttonFunVal = 0
jokeCount = 0
manualMode = 0
word_said = ""
text_to_speak = " Hello"

#---------------------------------Listener
def listen_for_command():
    recognizer = sr.Recognizer()
    global word_said
    global text_to_speak
    with sr.Microphone() as source:
        print("Listening...")
        audio = recognizer.listen(source)
        print("Not Listening")
    
    try: 
        word_said = recognizer.recognize_google(audio)
        #cybot.write(word_said.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        print("Text: "+ word_said)
        
    except:
        text_to_speak ="Sorry, I did not get that"
        word_said = "I don't know"

    analyze_command()
    respond()
    
def analyze_command():
    global word_said
    global text_to_speak

    if ' forward' in word_said:
        text_to_speak = "Driving Forward"
        forward()

    elif ' turn' and ' right' in word_said:
        text_to_speak = "Turning Right"
        turn_right()

    elif ' turn' and ' left' in word_said:
        text_to_speak = "Turning left"
        turn_left()

    elif ' backward' in word_said:
        text_to_speak = "Driving Backwards"
        backward()

    elif 'scan' in word_said:
        text_to_speak = "Scanning"
        scan()

    elif ' bathroom' in word_said:
        text_to_speak = "Finding Bathroom"
        find_bathroom()
    
    elif ' kitchen' in word_said:
        text_to_speak = "Finding Kitchen"
        find_kitchen_full()

    elif ' living room' in word_said:
        text_to_speak = "Finding Living Room"
        find_livingRoom()

    elif ' ready' or ' exit' in word_said:
        text_to_speak = "Finding the exit"
        find_exit()

    elif ' stop' or ' quit' or ' no' in word_said:
        text_to_speak = "Stopping"
        stop()

def respond():
    global text_to_speak
    #engine = pyttsx3.init()
    #set the rate it talks in words per minute
    #engine.setProperty('rate', 150)
    #voices = engine.getProperty('voices')
    #change voice to index 1 for female adn 0 for male
    #engine.setProperty('voice', voices[1].id)
    #engine.setProperty('volume', 3.0)
    #engine.say(text_to_speak)
    #engine.runAndWait()

#---------------------------------PLOT in POLAR
def plot():
    angle_degrees = [] # Initially empty
    angle_radians = [] # Initially empty
    distance = []      # Initially empty
    line = cybot.readline().decode().strip()  # Decode and strip newline characters
    while True: 
        #count += 1
        #if count % 2 == 0:
        line = cybot.readline().decode().strip()   # Split line into columns (by default delineates columns by whitespace)
        # print("Received line:", line)  # Debug print
        clean_line = re.sub(r'[\x00]', '', line)  # Remove specific unwanted characters
        data = clean_line.split() # splits the line based on white space
        # print("Received data:", data)  # Debug print
        angle = float(data[0])
        angle_degrees.append(float(data[0]))  # Column 0 holds the angle at which distance was measured
        distance.append(float(data[1])/100)       # Column 1 holds the distance that was measured at a given angle, also convert from cm to m 
        if angle == 180:    #break when lines are null
             break
                
    # Convert python sequence (list of strings) into a numpy array
    line = cybot.readline().decode().strip()   # Split line into columns (by default delineates columns by whitespace)
    angle_degrees = np.array(angle_degrees) # Avoid "TypeError: can't multiply sequence by non-int of type float"
                                        # Link for more info: https://www.freecodecamp.org/news/typeerror-cant-multiply-sequence-by-non-int-of-type-float-solved-python-error/
                                        
    angle_radians = (np.pi/180) * angle_degrees # Convert degrees into radians

# Create a polar plot
    fig, ax = plt.subplots(subplot_kw={'projection': 'polar'}) # One subplot of type polar
    ax.plot(angle_radians, distance, color='r', linewidth=4.0)  # Plot distance verse angle (in radians), using red, line width 4.0
    ax.set_xlabel('Distance (m)', fontsize = 14.0)  # Label x axis
    ax.set_ylabel('Angle (degrees)', fontsize = 14.0) # Label y axis
    ax.xaxis.set_label_coords(0.5, 0.15) # Modify location of x axis label (Typically do not need or want this)
    ax.tick_params(axis='both', which='major', labelsize=14) # set font size of tick labels
    ax.set_rmax(2.5)                    # Saturate distance at 2.5 meters
    ax.set_rticks([0.25, 0.5, 0.75, 1, 1.25, 1.5])   # Set plot "distance" tick marks at .5, 1, 1.5, 2, and 2.5 meters
    ax.set_rlabel_position(-22.5)     # Adjust location of the radial labels
    ax.set_thetamax(180)              # Saturate angle to 180 degrees
    ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4)) # Set plot "angle" tick marks to pi/4 radians (i.e., displayed at 45 degree) increments
  # Note: added .1 to pi to go just beyond pi (i.e., 180 degrees) so that 180 degrees is displayed
    ax.grid(True)            # Show grid lines

# Create title for plot (font size = 14pt, y & pad controls title vertical location) FIXME SHOW PLOT HERE
    ax.set_title("Mock-up Polar Plot of CyBot Sensor Scan from 0 to 180 Degrees", size=14, y=1.0, pad=-24) 
    plt.show()  # Display plot
#---------------------------------End of Plot

#---------------------------------PLOT in 2-D map
# Create a figure and axes
fig, ax = plt.subplots()
global x_values, y_values, x_values_short, y_values_short
#  for object locations
x_values = np.array([0])  # x coordinates of tall objects
y_values = np.array([0])  # y coordinates of tall objects
x_values_short = np.array([0])  # x coordinates of short objects
y_values_short = np.array([0])  # y coordinates of short objects

#Cybot values to track the cybot orientation 
global cybotx, cyboty, theta, distance, angle, direction
direction = "west"
cybotx= 0.20
cyboty = 0.20
theta = 0


#Adding 'rooms'
def plot_rooms():
    global cybotx, cyboty
    #Bedroom
    bedroom = patches.Rectangle((0,0), width=.61, height=.61, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(bedroom)
    #Bathroom
    bathroom = patches.Rectangle((3.67,0), width=.61, height=.61, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(bathroom)
    #Living Room
    living_room = patches.Rectangle((3.67,1.79), width=.61, height=.61, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(living_room)
    #Kitchen
    kitchen = patches.Rectangle((0,1.79), width=.61, height=.61, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(kitchen)
    #Stairwell
    stairwell = patches.Rectangle((1.835,0.92), width=.60, height=.60, linewidth=4, edgecolor='black', facecolor='black')
    ax.add_patch(stairwell)
    #cybot perim
    cybotPerim = patches.Circle((cybotx, cyboty), radius=.14, linewidth=4, edgecolor='orange', facecolor='none')
    ax.add_patch(cybotPerim)


def plot_config():

    #Set Size of Plot
    ax.set_xlim(0,4.28)
    ax.set_ylim(0,2.40)

    #Arranging Grid Size
    x_gridlines = np.arange(0,4.28,0.61)
    y_gridlines = np.arange(0,2.40,0.61)
    ax.set_xticks(x_gridlines)
    ax.set_yticks(y_gridlines)

    #labels for plot
    ax.set_title('2D plot example')
    ax.set_xlabel('X-axis')
    ax.set_ylabel('Y-axis')

    #Turn on grid lines
    ax.grid(True)

# init plot
plot_rooms()
plot_config()
#plot points for tall objects
#ax.scatter(x_values, y_values, color='blue', label='Data Points', s=100)
#plot points for short objects
#ax.scatter(x_values_short, y_values_short, color='red', label='Data Points2', s=100)
#plot cybot
ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='>', s=125)

#interactive mode to update plot dynamically
plt.ion()

#display the plot
plt.show()

def update_plot():
    global cybotx
    global cyboty
    global x_values, y_values, x_values_short, y_values_short
    #print("In function " + str(cybotx) + "\n")
    #clear current plot
    ax.clear() 
    #plot tall objects
    ax.scatter(x_values, y_values, color='blue', label='Data Points', s=300)
    #plot Short objects
    ax.scatter(x_values_short, y_values_short, color='red', label='Data Points2', s=300)
    #plot cybot depending on direction it is facing
    match direction:
        case "north":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='v', s=200)
        case "south":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='^', s=200)
        case "east":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='<', s=200)
        case "west":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='>', s=200)
        case "northwest":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='^', s=200)
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='<', s=200)
        case "southwest":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='v', s=200)
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='<', s=200)
        case "northeast":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='^', s=200)   
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='>', s=200)
        case "southeast":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='v', s=200)
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='>', s=200)


    #add 'rooms'
    plot_rooms()
    #add config
    plot_config()
    #redraw plot
    plt.draw()
    plt.pause(0.5)
    #print("Redrawing plot " + str(cybotx) + "\n")

def add_object():
    #hopefully adding any object that the cybot "sees", which is a tall object 
    global cyboty, cybotx, theta, distance, angle
    #read in the data 
    # Split line into columns (by default delineates columns by whitespace)
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line) 
    # splits the line based on white space
    data = clean_line.split() 
    angle = float(data[0])
    distance = float(data[1])/100
    #find coordinates of new object
    x_coord = cybotx + distance*np.cos(np.deg2rad(angle + theta))
    y_coord = cybotx + distance*np.sin(np.deg2rad(angle + theta))
    #append to array
    x_values = np.append(x_values, x_coord)
    y_values = np.append(y_values, y_coord)
#---------------------------------END of PLOT in 2-D map

#---------------------------------Sending Commands
def send_msg():
    global count
    send_message = input("Enter a message (enter quit to exit):") + "\n" # Enter next message to send to server
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    print("Sent to server: " + send_message) 
    target_letter = b'\x00m\n'
    print("wait for server reply\n")
    rx_message = cybot.readline()      # Wait for a message, readline expects message to end with "\n"
    #print("Received message:", repr(rx_message))
    print("Got a message from server: " + rx_message.decode() + "\n") # Convert message from bytes to String (i.e., decode)
    if  rx_message == target_letter: 
        print("message was m or h\n") #want to print the graph
        plot() #def on line 97
    movement = {
        "north" : (0, -0.5),
        #"northwest"
        "west" : (0.5, 0),
        "east" : (-0.5, 0),
        "south" : (0, 0.5)
    }
    global cybotx, cyboty, direction, flag_updateGraph, buttonFunVal
    match send_message:
        case "w\n":
            dx, dy = movement[direction]
            cybotx += dx
            cyboty += dy
        case "s\n":
            dx, dy = movement[direction]
            cybotx += dx
            cyboty += dy
        case "a\n":
            direction ={
                "north" : "west",
                "west" : "south",
                "south" : "east",
                "east" : "north",
            }[direction]
        case "d\n":
            direction ={
                "north" : "east",
                "west" : "north",
                "south" : "west",
                "east" : "south",
            }[direction]
    print("X val: " + str(cybotx) + "Y val: " + str(cyboty) + "Direction: "+ direction)
    buttonFunVal = 0
    flag_updateGraph = 1
    #update_plot()
        
#--------------------------------- Basic Driving functions
def forward():
    global flag_updateGraph
    send_message = "w\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Got a message from server: " + clean_line + "\n") # Convert message from bytes to String (i.e., decode)
    #update_plot()
    flag_updateGraph = 1  

def turn_right():
    global flag_updateGraph
    send_message = "d\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    flag_updateGraph = 1 
    #update_plot()   

def turn_left():
    global flag_updateGraph
    send_message = "a\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server 
    flag_updateGraph = 1 
    #update_plot() 

def backward():
    global flag_updateGraph
    send_message = "s\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server 
    flag_updateGraph = 1 
    #update_plot()   

def stop():
    global eStop, flag_updateGraph
    eStop = 1
    send_message = "t\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Got a message from server: " + clean_line + "\n") # Convert message from bytes to String (i.e., decode)
    flag_updateGraph = 1 
    #update_plot()   

def scan():
    send_message = "m\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server   
    target_letter = b'\x00m\n'
    print("wait for server reply\n")
    rx_message = cybot.readline()      # Wait for a message, readline expects message to end with "\n"
    #print("Received message:", repr(rx_message))
    print("Got a message from server: " + rx_message.decode() + "\n") # Convert message from bytes to String (i.e., decode)
    if  rx_message == target_letter: 
        print("message was m or h\n") #want to print the graph
        plot()   #def on line 97

def transform_heading_angle(input_angle):
    """Previous heading angle transformation function remains the same"""
    input_angle = input_angle % 360
    
    direction_map = {
        80.33: 90,    # North
        125: 45,      # Northeast
        159: 0,       # East
        204: 315,     # Southeast
        250: 270,    # South
        305: 225,    # Southwest
        349: 180,    # West
        38: 135      # Northwest
    }
    
    closest_angle = min(direction_map.keys(), 
                       key=lambda x: min(abs(input_angle - x), 
                                       abs(input_angle - (x + 360)), 
                                       abs(input_angle - (x - 360))))
    
    return direction_map[closest_angle]

def autoTakingScanData():
    global flag_updateGraph
    line = cybot.readline().decode().strip() 
    count = 0
    cybotToServo = .105
    servoToPing = .04
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Received message " + clean_line + "\n") 
    #plot()
    while 'finished' not in clean_line:
        #Must be sending data on an object in format : Distance \t angle
        #split data from line into variables
        try:
            global cybotx, cyboty, x_values, y_values
            data = clean_line.split()
            pingToObject = float(data[0])/100 #FIXME CM OR MM
            servo_angle = int(data[1])
            heading = int(data[2])
            count += 1
            print("Distance: " + str(pingToObject) + " Angle: " + str(servo_angle) + " Heading: " + str(heading) )
            #map the heading using the direction key
            transformed_heading = transform_heading_angle(heading)
            heading_rad = np.deg2rad(transformed_heading)

            # Calculate servo position
            servo_pos_x = cybotx + cybotToServo * np.cos(heading_rad)
            servo_pos_y = cyboty + cybotToServo * np.sin(heading_rad)

            # Flip the servo angle (180 - servo_angle) and adjust relative to heading
            adjusted_servo_angle = (180 - servo_angle)
            total_angle = (transformed_heading + (90 - adjusted_servo_angle)) % 360
            total_angle_rad = np.deg2rad(total_angle)

            # Calculate object position (servoToPing + pingToObject is total distance from servo to object)
            total_distance = servoToPing + pingToObject
            object_x = servo_pos_x + total_distance * np.cos(total_angle_rad)
            object_y = servo_pos_y + total_distance * np.sin(total_angle_rad)
                
            #append to array
            x_values = np.append(x_values, object_x)
            y_values = np.append(y_values, object_y)
            flag_updateGraph = 1
            #update_plot()
        finally:
            line = cybot.readline().decode().strip() 
            # Remove specific unwanted characters  
            clean_line = re.sub(r'[\x00]', '', line)

def takingScanData(rx_message):
    global flag_updateGraph
    #line = cybot.readline().decode().strip() 
    count = 0
    cybotToServo = .105
    servoToPing = .04
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', rx_message)
    #print("Received message " + clean_line + "\n") 
    #plot()
    global direction
    directionAngle = {
        "north" : 270,
        "northeast" : 225,
        "northwest" : 315,
        "west" : 0,
        "east" : 180,
        "south" : 90,
        "southeast" : 135,
        "southwest" : 45,
    }
    while 'finished' not in clean_line:
        #Must be sending data on an object in format : Distance \t angle
        #split data from line into variables
        try:
            global cybotx, cyboty, x_values, y_values, direction
            data = clean_line.split()
            pingToObject = round(float(data[0])/100, 2) #FIXME CM OR MM
            servo_angle = int(data[1])
            oppositeDistofBot = float(data[2])
            count += 1
            print("Distance: " + str(pingToObject) + "  Angle: " + str(servo_angle) + "  ObjOppDist: " + str(oppositeDistofBot) + "\n")
            #map the heading using the direction key
            #transformed_heading = transform_heading_angle(heading)
            #heading_rad = np.deg2rad(transformed_heading)
            #position facing based on what python code thinks
            position = directionAngle[direction]
            position_rad = np.deg2rad(position)
            # Calculate servo position
            servo_pos_x = cybotx + cybotToServo * np.cos(position_rad)
            servo_pos_y = cyboty + cybotToServo * np.sin(position_rad)

            # Flip the servo angle (180 - servo_angle) and adjust relative to heading
            adjusted_servo_angle = (180 - servo_angle)
            total_angle = (position + (90 - adjusted_servo_angle)) % 360
            total_angle_rad = np.deg2rad(total_angle)

            # Calculate object position (servoToPing + pingToObject is total distance from servo to object)
            total_distance = servoToPing + pingToObject
            object_x = servo_pos_x + total_distance * np.cos(total_angle_rad)
            object_y = servo_pos_y + total_distance * np.sin(total_angle_rad)
            #print("X: " + str(object_x) + " Y:" + str(object_y))

            #append to array
            x_values = np.append(x_values, object_x)
            y_values = np.append(y_values, object_y)
            #flag_updateGraph = 1
            update_plot()
        finally:
            line = cybot.readline().decode().strip() 
            # Remove specific unwanted characters  
            clean_line = re.sub(r'[\x00]', '', line)
    
def forwardDetectData():
    rx_message = cybot.readline()   # Wait for a message, readline expects message to end with "\n"
    #print("Received message:", repr(rx_message))
    print("Got a message from server: " + rx_message.decode() ) # Convert message from bytes to String (i.e., decode)
    cleanMsg = rx_message.decode().strip()
    movement = {
        "north" : (0, -1),
        "northwest" : (.7071, -.7071),
        "northeast" : (-.7071, -.7071),
        "west" : (1, 0),
        "east" : (-1, 0),
        "south" : (0, 1),
        "southeast": (-.7071, .7071),
        "southwest":(.7071, .7071)
    }
    while 'done' not in cleanMsg:
        global cybotx, cyboty, direction
        global flag_updateGraph
        #print("In Loop Received message " + clean_line + "\n")
        #Check if a bump occured 
        if 'bump right' in cleanMsg:
            bumpRight()
            update_plot()
        elif 'bump left' in cleanMsg:
            bumpLeft() 
            update_plot() 
        elif 'moved' in cleanMsg:
            data = cleanMsg.split() # splits the line based on white space
            print("Received data:", data)  # Debug print
            dist = float(data[1])/1000
            #find which direction we are currently pointing
            dx, dy = movement[direction]
            cybotx += dx*dist
            cyboty += dy*dist
            flag_updateGraph = 1 
        elif 'wall' in cleanMsg:
            #the cliff sensor was triggered 
            print("A wall")
            if 'both' in cleanMsg:
                match direction:
                    case "north":
                        #we want the cybot to turn CCW, must be at bottom border
                        cyboty = 0.10
                    case "south":
                        #WTF turn CW I guess
                        cyboty = 2.34
                    case "east":
                        #Turn 2 CCW
                        cybotx = 0.10
                    case "west":
                        #Turn CW
                        cybotx = 4.17
            update_plot()
        elif 'hole' in cleanMsg:
            #the cliff sensor was triggered 
            match direction:
                case "north":
                    cyboty = 1.525
                case "south":
                    #turn CW I guess
                    cyboty = .915
                case "east":
                    #Turn 2 CCW
                    cybotx = 2.44
                case "west":
                    #Turn CW
                    cybotx = 1.83
            if 'both' in cleanMsg:
                print("Hit hole on both sides\n")
            elif 'right' in cleanMsg:
                print("right hole only\n")
            elif 'left' in cleanMsg:
                print("left hole only\n")
            flag_updateGraph = 1 
        rx_message = cybot.readline()   # Wait for a message, readline expects message to end with "\n"
        cleanMsg = rx_message.decode().strip()
        print("Received: " + cleanMsg)
        
def manual():
    #send_message = input("MANUAL MODE: Enter a message (enter quit to exit):")  # Enter next message to send to server
    #cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    #print("Sent to server: " + send_message) 
    #rx_message = cybot.readline()   # Wait for a message, readline expects message to end with "\n"
    #print("Received message:", repr(rx_message))
    #print("Got a message from server: " + rx_message.decode() ) # Convert message from bytes to String (i.e., decode)
    #cleanMsg = rx_message.decode().strip()
    movement = {
        "north" : (0, -1),
        "northwest" : (.7071, -.7071),
        "northeast" : (-.7071, -.7071),
        "west" : (1, 0),
        "east" : (-1, 0),
        "south" : (0, 1),
        "southeast": (-.7071, .7071),
        "southwest":(.7071, .7071)
    }
    #cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    while 1:
        global cybotx, cyboty, direction
        global flag_updateGraph
        send_message = input("Enter a message (enter quit to exit):") + '\n' # Enter next message to send to server
        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        rx_message = cybot.readline().decode().strip()   # Wait for a message, readline expects message to end with "\n"
        #print("Received message:", repr(rx_message))
        print("Got a message from server: " + rx_message) # Convert message from bytes to String (i.e., decode)
        print("Msg Sent: " + send_message + "Received message:"+ rx_message)

        match send_message:
            case "w\n":
                #rx_message = cybot.readline()   # Wait for a message, readline expects message to end with "\n"
                print("Received message:"+ rx_message)
                dist = input("Enter how far forward in centimeters you want to go:") + '\n'
                #options = input("2) 200 3)300 5) 500: ")
                #case ""
                cybot.write(dist.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                #rx_message = cybot.readline()   # Wait for a message, readline expects message to end with "\n"
                #print("Received message:" + rx_message.decode().strip())
                forwardDetectData()
            case "s\n":
                #rx_message = cybot.readline()   # Wait for a message, readline expects message to end with "\n"
                print("Received message:"+ rx_message)
                dist = input("Enter how far forward in centimeters you want to go:") + '\n'
                cybot.write(dist.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                rx_message = cybot.readline()   # Wait for a message, readline expects message to end with "\n"
                print("Received message:"+ rx_message.decode().strip())
                try:
                    #print("Received message:"+ rx_message.decode().strip().split())
                    data = rx_message.decode().strip().split()
                    distance = float(data[1])/1000
                    dx, dy = movement[direction]
                    cybotx += dx*distance
                    cyboty += dy*distance 
                except ValueError:
                    print("Invalid input. Please enter a valid number.")
                
            case "a\n":
                direction ={
                    "north" : "west",
                    "northwest": "southwest",
                    "northeast" : "northwest",
                    "west" : "south",
                    "south" : "east",
                    "southeast" : "northeast",
                    "southwest" : "southeast",
                    "east" : "north",
                }[direction]
                print("updated dir: " + direction + "\n")
            case "q\n":
                direction ={
                    "north" : "northwest",
                    "northwest" : "west",
                    "west" : "southwest",
                    "southwest" : "south",
                    "south": "southeast",
                    "southeast": "east",
                    "east": "northeast",
                    "northeast": "north",
                }[direction]
                print("updated dir: " + direction + "\n")
            case "d\n":
                direction ={
                    "north" : "east",
                    "northeast" : "southeast",
                    "northwest" : "northeast",
                    "west" : "north",
                    "south" : "west",
                    "southwest" : "northwest",
                    "southeast" : "southwest",
                    "east" : "south",
                }[direction]
                print("updated dir: " + direction + "\n")
            case "e\n":
                direction ={
                    "north" : "northeast",
                    "northeast" : "east",
                    "east" : "southeast",
                    "southeast" : "south",
                    "southwest" : "west",
                    "south" : "southwest",
                    "west" : "northwest",
                    "northwest": "north",
                }[direction]
                print("updated dir: " + direction + "\n")
            case "m\n":
                plot()
                rx_message = cybot.readline().decode().strip() 
                takingScanData(str(rx_message))
            case "f\n":
                takingScanData(str(rx_message))
            case "g\n":
                takingScanData(str(rx_message))
        print("MANUAL: X val: " + str(cybotx) + " Y val: " + str(cyboty) + " Direction: "+ direction + "\n")
        flag_updateGraph = 1 
        update_plot()
        
def setLoc():
    global cybotx, cyboty
    global flag_updateGraph
    cybotx = 3.75
    cyboty = .57
    flag_updateGraph = 1 
    #update_plot()
#--------------------------------- Cybot Tracking Functions     
def bumpLeft():
    #the cybot hit an object with the left bump sensor 
    global  cybotx, cyboty, x_values_short, y_values_short
    global flag_updateGraph
    #Need to recieve the distance traveled from the cybot in order to pinpoint object location ONLY NEED X DIRECTION
    #line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    #clean_line = re.sub(r'[\x00]', '', line) 
    #Object is right to the left of the cybot 
    #add values to the array
    x_values_short = np.append(x_values_short, cybotx)
    y_values_short = np.append(y_values_short, cyboty)
    #update cybot location // only y direction though??? FIXME
    flag_updateGraph = 1 
    #update_plot()
    #Now cybot will scan to the right to see if it is an ideal move path???
    # FIXME 

def bumpRight():
    #the cybot hit an object with the left bump sensor 
    global flag_updateGraph
    global cybotx, cyboty, x_values_short, y_values_short
    #Need to recieve the distance traveled from the cybot in order to pinpoint object location ONLY NEED X DIRECTION
    #line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    #clean_line = re.sub(r'[\x00]', '', line) 
    #Object is right to the left of the cybot 
    #add values to the array
    x_values_short = np.append(x_values_short, cybotx)
    y_values_short = np.append(y_values_short, cyboty)
    #update cybot location
    flag_updateGraph = 1 
    #update_plot()
    #Now cybot will scan to the left to see if it is an ideal move path???
    # FIXME  

def turnedClockwise():
    global direction
    direction ={
                "north" : "east",
                "west" : "north",
                "south" : "west",
                "east" : "south",
            }[direction]

def turnedCounterClockwise():
    global direction
    direction ={
                "north" : "west",
                "west" : "south",
                "south" : "east",
                "east" : "north",
            }[direction]

def find_bathroom():
    global buttonFunVal
    # this function should move the bot from the bottom left to the bottom right 
    send_message = "1\n" 
    eStop = 0 
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    time.sleep(0.5)
    #print("sent message: " + send_message) 
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Received message " + clean_line + "\n") 
    progress = 'no'
    movement = {
        "north" : (0, -0.5),
        "west" : (0.5, 0),
        "east" : (-0.5, 0),
        "south" : (0, 0.5)
    }
    while 'done' not in progress:
        global cybotx, cyboty, direction, jokeCount, manualMode
        global flag_updateGraph
        #print("In Loop Received message " + clean_line + "\n")
        #Check if a bump occured 
        if manualMode == 1:
            break
        if 'bump right' in clean_line:
            bumpRight()
            jokeCount += 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'bump left' in clean_line:
            bumpLeft()
            jokeCount += 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'scanning' in clean_line:
            autoTakingScanData()
            flag_updateGraph = 1 
            #update_plot()
            #send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            flag_updateGraph = 1 
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            flag_updateGraph = 1 
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'moved' in clean_line:
            data = clean_line.split() # splits the line based on white space
            print("Received data:", data)  # Debug print
            dist = float(data[1])/1000
            #find which direction we are currently pointing
            match direction:
                case "north":
                    cyboty -= dist
                case "south":
                    cyboty += dist
                case "east":
                    cybotx -= dist
                case "west":
                    cybotx += dist
            flag_updateGraph = 1 
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'wall' in clean_line:
            #the cliff sensor was triggered 
            print("A wall")
            match direction:
                case "north":
                    #we want the cybot to turn CCW, must be at bottom border
                    cyboty = 0.10
                    send_message = "left\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #WTF turn CW I guess
                    cyboty = 2.34
                    jokeCount += 2
                    send_message = "right\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn 2 CCW
                    cybotx = 0.10
                    jokeCount += 1
                    send_message = "2\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn CW
                    #cybotx = 4.17
                    jokeCount += 1
                    send_message = "right\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
            #update_plot()
        elif 'hole' in clean_line:
            #sending data back -- "left" is CCW -- "right" is CW -- "2" is 2 turns 
            print("A hole")
            match direction:
                case "north":
                    cyboty = 1.525
                    #we want the cybot to turn CCW,
                    send_message = "left\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #turn CW I guess
                    cyboty = .915
                    jokeCount += 2
                    send_message = "right\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn 2 CCW
                    cybotx = 2.44
                    jokeCount += 1
                    send_message = "2\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn CW
                    cybotx = 1.83
                    jokeCount += 1
                    send_message = "right\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
        elif 'stop' in clean_line:
            eStop = 1
            manualMode = 1
            break
        if cybotx >= 3.70:
            if cyboty <= .61 and 'west' in direction:
                progress = 'done'
                send_message = "f\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                break
            elif cyboty <= .61 and 'west' not in direction:
                match direction:
                    case "north":
                        #turn 1 CCW
                        send_message = "left\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "south":
                        #turn 1 CW
                        jokeCount += 1
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "east":
                        #turn 2 CW
                        jokeCount += 2
                        send_message = "2\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cyboty >= .61 and 'north' in direction:
                #Just Drive forward
                send_message = "0\n"
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cyboty >= .61 and 'north' not in direction:
                match direction:
                    case "south":
                        #turn  2 CW
                        send_message = "2\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "west":
                        #turn 1 CW
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "east":
                        #turn 1 CCW
                        send_message = "left\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        if eStop == 1:
            break
        if jokeCount == 8:
            send_message = "j\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server 
        print("B) X: " + str(cybotx) + " Y: " + str(cyboty) + "Dir: " + direction)
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
        print("Heard: " + clean_line)
    #update_plot()
    buttonFunVal = 0
    #else: FIXME 
    #    send_message = "g\n"    
    #    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server 

def findOrientation():
    global direction
    if "west" in direction:
        send_message = "2\n"  
        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

def find_kitchen_step1():
    #Move across near the bedroom
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    progress = 'no'
    while 'done' not in progress:
        global cybotx, cyboty, direction, flag_updateGraph, jokeCount
        #Check if a bump occured 
        if 'bump right' in clean_line:
            bumpRight()
            jokeCount += 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'bump left' in clean_line:
            bumpLeft()
            jokeCount += 1 
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'scanning' in clean_line:
            autoTakingScanData()
            flag_updateGraph = 1 
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            flag_updateGraph = 1 
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            #update_plot()
            flag_updateGraph = 1 
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'moved' in clean_line:
            data = clean_line.split() # splits the line based on white space
            print("Received data:", data)  # Debug print
            dist = float(data[1])/1000
            #find which direction we are currently pointing
            match direction:
                case "north":
                    cyboty -= dist
                case "south":
                    cyboty += dist
                case "east":
                    cybotx -= dist
                case "west":
                    cybotx += dist
            flag_updateGraph = 1
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'wall' in clean_line:
            #sending data back -- "left" is CCW -- "right" is CW -- "2" is 2 turns 
            print("A wall")
            match direction:
                case "north":
                    #we want the cybot to turn CW, must be at bottom border
                    cyboty = 0.10
                    send_message = "right\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #turn CCW I guess
                    cyboty = 2.34
                    jokeCount += 2
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn  CW
                    cybotx = 0.10
                    jokeCount += 1
                    send_message = "right\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn 2 CW
                    cybotx = 4.17
                    jokeCount += 1
                    send_message = "2\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
            #update_plot()
        elif 'hole' in clean_line:
            #sending data back -- "left" is CCW -- "right" is CW -- "2" is 2 turns 
            print("A hole")
            match direction:
                case "north":
                    #we want the cybot to turn CW
                    cyboty = 1.525
                    send_message = "right\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #turn CCW I guess
                    cyboty = .915
                    jokeCount += 2
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn CW
                    cybotx = 2.44
                    jokeCount += 1
                    send_message = "right\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn CCW
                    cybotx = 1.83
                    jokeCount += 1
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
            #update_plot()
        if cybotx <= 0.90:
            if 'east' in direction:
                send_message = "f\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                progress = 'done'
            else:
                match direction:
                    case "north":
                        #turn 1 CW
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "west":
                        #turn 2 CW
                        send_message = "2\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "south":
                        #turn 1 CCW
                        send_message = "left\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        #update_plot()
        flag_updageGraph = 1
        print("K1) X: " + str(cybotx) + " Y: " + str(cyboty) + "Dir: " + direction)
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
        print("Heard: " + clean_line)
    send_message = "f\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

def find_kitchen_full():
    # this function should move the bot from the bottom right to the top left 
    send_message = "2\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    findOrientation()
    #I split it into 2 steps to make the code more readable, step 1 is a different function
    find_kitchen_step1()
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    progress = 'no'
    while 'done' not in progress:
        global cybotx, cyboty, direction, flag_updateGraph, jokeCount
        #Check if a bump occured 
        if 'bump right' in clean_line:
            bumpRight()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'bump left' in clean_line:
            bumpLeft()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'scanning' in clean_line:
            autoTakingScanData()
            #update_plot()
            flag_updateGraph = 1 
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            #update_plot()
            flag_updateGraph = 1 
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            #update_plot()
            flag_updateGraph = 1 
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'moved' in clean_line:
            data = clean_line.split() # splits the line based on white space
            print("Received data:", data)  # Debug print
            dist = float(data[1])/1000
            #find which direction we are currently pointing
            match direction:
                case "north":
                    cyboty -= dist
                case "south":
                    cyboty += dist
                case "east":
                    cybotx -= dist
                case "west":
                    cybotx += dist
            #update_plot()
            flag_updateGraph = 1 
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'wall' in clean_line:
            #sending data back -- "left" is CCW -- "right" is CW -- "2" is 2 turns 
            print("A wall")
            match direction:
                case "north":
                    #we want the cybot to turn 2 CW, must be at bottom border
                    cyboty = 0.10
                    send_message = "2\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #turn CCW I guess
                    cyboty = 2.34
                    jokeCount += 2
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn  CW
                    cybotx = 0.10
                    jokeCount += 1
                    send_message = "right\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn 2 CW
                    cybotx = 4.17
                    jokeCount += 1
                    send_message = "2\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
            #update_plot()
        elif 'hole' in clean_line:
            #sending data back -- "left" is CCW -- "right" is CW -- "2" is 2 turns 
            print("A hole")
            match direction:
                case "north":
                    cyboty = 1.525
                    #we want the cybot to turn CCW,
                    send_message = "left\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #turn CCW I guess
                    cyboty = .915
                    jokeCount += 2
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn CW
                    cybotx = 2.44
                    jokeCount += 1
                    send_message = "right\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn CCW
                    cybotx = 1.83
                    jokeCount += 1
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
        if cyboty >= 1.75:
            if cybotx > 0.75 and 'east' in direction:
                #drive forward
                send_message = "0\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cybotx > 0.75 and 'east' not in direction:
                match direction:
                    case "north":
                        #turn 1 CW
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "west":
                        #turn 2 CW
                        send_message = "2\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "south":
                        #turn 1 CCW
                        send_message = "left\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cybotx <= 0.75 and 'south' in direction:
                progress = 'done'
                send_message = "f\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cybotx <= 0.75 and 'south' not in direction:
                match direction:
                    case "north":
                        #turn 2 CW
                        send_message = "2\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "west":
                        #turn 1 CCW
                        send_message = "left\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "east":
                        #turn 1 CW
                        send_message = "right\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        #update_plot()
        flag_updateGraph = 1 
        print("K2) X: " + str(cybotx) + " Y: " + str(cyboty) + "Dir: " + direction)
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
        print("Heard: " + clean_line)
    send_message = "f\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

def find_livingRoom():
     # this function should move the bot from the top left to the top right 
    send_message = "3\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    time.sleep(0.5)
    #print("sent message: " + send_message) 
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Received message " + clean_line + "\n") 
    progress = 'no'
    movement = {
        "north" : (0, -0.5),
        "west" : (0.5, 0),
        "east" : (-0.5, 0),
        "south" : (0, 0.5)
    }
    while 'done' not in progress:
        global cybotx, cyboty, direction, jokeCount, manualMode
        global flag_updateGraph
        #Check if a bump occured 
        if 'bump right' in clean_line:
            bumpRight()
            jokeCount += 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'bump left' in clean_line:
            bumpLeft()
            jokeCount += 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'scanning' in clean_line:
            autoTakingScanData()
            flag_updateGraph = 1
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            flag_updateGraph = 1
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            flag_updateGraph = 1
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'moved' in clean_line:
            data = clean_line.split() # splits the line based on white space
            print("Received data:", data)  # Debug print
            dist = float(data[1])/1000
            #find which direction we are currently pointing
            match direction:
                case "north":
                    cyboty -= dist
                case "south":
                    cyboty += dist
                case "east":
                    cybotx -= dist
                case "west":
                    cybotx += dist
            flag_updateGraph = 1
            #update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'wall' in clean_line:
            #the cliff sensor was triggered by wall/outer boundary
            print("wall")
            match direction:
                case "north":
                    #we want the cybot to turn CCW, must be at bottom border
                    cyboty = 0.10
                    send_message = "left\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #at top boundary needs to turn CW
                    cyboty = 2.34
                    jokeCount += 1
                    send_message = "right\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn 2 CCW
                    cybotx = 0.1
                    jokeCount += 1
                    send_message = "2\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn CCW
                    cybotx = 4.15
                    jokeCount += 1
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
            #update_plot()
        elif 'hole' in clean_line:
            #the cliff sensor was triggered by black hole
            print("hole")
            match direction:
                case "north":
                    #we want the cybot to turn CCW, must be at bottom border
                    cyboty = 1.515
                    send_message = "left\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #at top boundary needs to turn CW 
                    cyboty = .915                
                    jokeCount += 1
                    send_message = "right\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn 2 CCW/turn around to face west
                    cybotx = 2.44                 
                    jokeCount += 1
                    send_message = "2\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn CCW to face south 
                    cybotx = 1.83           
                    jokeCount += 1
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
            #update_plot()
        elif 'stop' in clean_line:
            eStop = 1
            manualMode = 1
            break
        if cybotx >= 3.65:
            if cyboty >= 1.83 and 'west' in direction:
                progress = 'done'
                send_message = "f\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                break
            elif cyboty >= 1.83 and 'west' not in direction:
                match direction:
                    case "north":
                        #turn 1 CCW
                        send_message = "left\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "south":
                        #turn 1 CW
                        jokeCount += 1
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "east":
                        #turn 2 CW
                        jokeCount += 1
                        send_message = "2\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cyboty <= 1.83 and 'south' in direction:
                #Just Drive forward
                send_message = "0\n"
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server            
            elif cyboty <= 1.83 and 'south' not in direction:
                match direction:
                    case "north":
                        #turn  2 CW
                        send_message = "2\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "west":
                        #turn 1 CCW
                        send_message = "left\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "east":
                        #turn 1 CW
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        if eStop == 1:
            break
        if jokeCount == 8:
            send_message = "j\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server     
        print("X: " + str(cybotx) + " Y: " + str(cyboty) + "Dir: " + direction)
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
    #send_message = "1\n"  
    #cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

def find_exit():
     # this function should move the bot from the top left to the top right 
    send_message = "4\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    time.sleep(0.5)
    print("sent message: " + send_message) 
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Received message " + clean_line + "\n") 
    progress = 'no'
    while 'done' not in progress:
        global cybotx, cyboty, direction, update_flagGraph, jokeCount
        #Check if a bump occured 
        if 'bump right' in clean_line:
            bumpRight()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'bump left' in clean_line:
            bumpLeft()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'scanning' in clean_line:
            autoTakingScanData()
            #update_plot()
            update_flagGraph = 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            #update_plot()
            update_flagGraph = 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            #update_plot()
            update_flagGraph = 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'moved' in clean_line:
            data = clean_line.split() # splits the line based on white space
            print("Received data:", data)  # Debug print
            dist = float(data[1])/1000
            #find which direction we are currently pointing
            match direction:
                case "north":
                    cyboty -= dist
                case "south":
                    cyboty += dist
                case "east":
                    cybotx -= dist
                case "west":
                    cybotx += dist
            #update_plot()
            update_flagGraph = 1
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'wall' in clean_line:
            #sending data back -- "left" is CCW -- "right" is CW -- "2" is 2 turns 
            print("A wall")
            match direction:
                case "north":
                    #we want the cybot to turn 2 CW, must be at bottom border
                    cyboty = 0.10
                    send_message = "2\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #turn 2 CCW I guess
                    cyboty = 2.34
                    jokeCount += 2
                    send_message = "2\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn  2CW
                    cybotx = 0.10
                    jokeCount += 1
                    send_message = "2\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    if cyboty <= 1:
                        #Turn  CCW
                        cybotx = 4.17
                        jokeCount += 1
                        send_message = "left\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    else: 
                        #Turn  CW
                        cybotx = 4.17
                        jokeCount += 1
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
            #update_plot()
        elif 'hole' in clean_line:
            #sending data back -- "left" is CCW -- "right" is CW -- "2" is 2 turns 
            print("A hole")
            match direction:
                case "north":
                    #we want the cybot to turn CW
                    cyboty = 1.525
                    send_message = "right\n"  
                    jokeCount += 1
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "south":
                    #turn CW I guess
                    cyboty = .915
                    jokeCount += 2
                    send_message = "right\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "east":
                    #Turn 2 CW
                    cybotx = 2.44
                    jokeCount += 1
                    send_message = "2\n"
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                case "west":
                    #Turn CCW
                    cybotx = 1.83
                    jokeCount += 1
                    send_message = "left\n"  
                    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            flag_updateGraph = 1 
        if cyboty >= 1.0 and cyboty <= 1.75:
            if cybotx >= 3.75:
                progress = 'done'
                send_message = "f\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cybotx < 3.75:
                match direction:
                    case "north":
                        #we want the cybot to turn CCW,
                        send_message = "left\n"  
                        jokeCount += 1
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "south":
                        #turn CW I guess
                        jokeCount += 2
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "east":
                        #Turn 2 CW
                        jokeCount += 1
                        send_message = "2\n"
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            #FIXME Finish this
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
    send_message = "f\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
# def what will take place in the thread
def socketThread():
    global manualMode,count
    while manualMode != 1: 
        global buttonFunVal
        match buttonFunVal:
            case "1":
                find_bathroom()
            case "2":
                find_kitchen_full()
            case "3":
                find_livingRoom()
            case "4":
                find_exit()
            case "L":
                listen_for_command()
            case "R":
                respond()
            case "m":
                send_msg()
    stop()
    manual()     

#Buttons will be used to decide which function will occur in the thread
#Here are the buttons making that decision

def buttonPush():
  #run the listener function
  global buttonFunVal,count
  buttonFunVal = "L"
  count = 1

def buttonPush2():
  #run the To Living Room function: find_livingRoom
  global buttonFunVal, count
  buttonFunVal = "3"
  count = 1
  
def buttonPush3():
  #run the To Exit
  global buttonFunVal, count
  buttonFunVal = "4"
  count = 1

def buttonPush4():
  #run the To Bathroom function: find_bathroom
  global buttonFunVal, count
  buttonFunVal = "1"
  count = 1

def buttonPush5():
  #run the To Kitchen function: find_kitchen_full
  global buttonFunVal,count 
  buttonFunVal = "2"
  count = 1

def buttonPush6():
  #run the responder function: respond
  global buttonFunVal, count
  buttonFunVal = "R"
  count = 1

def buttonPush7():
  #run the Send_msg function
  global buttonFunVal, count
  buttonFunVal = "m"
  count = 1

def buttonPush8():
  #run the STOP function
  global manualMode
  manualMode = 1
  
#creating buttons Widget
mybutton = Button(root, text="Listen", command=buttonPush, bg="red", fg="black", font=("Helvetica", 25)) #def listen_for_command
mybutton2 = Button(root, text="To Living Room", command=buttonPush2, bg="red", fg="black", font=("Helvetica", 25)) #def find_livingRoom
mybutton3 = Button(root, text="To Exit", command=buttonPush3, bg="red", fg="black", font=("Helvetica", 25)) #def find_exit
mybutton4 = Button(root, text="To Bathroom", command=buttonPush4, bg="red", fg="black", font=("Helvetica", 25)) #def find_bathroom
mybutton5 = Button(root, text="To Kitchen", command=buttonPush5, bg="red", fg="black", font=("Helvetica", 25)) #def find_kitchen_full
mybutton6 = Button(root, text="Manual", command=manual, bg="red", fg="black", font=("Helvetica", 25)) # respond
mybutton7 = Button(root, text="Send Msg", command=buttonPush7, bg="red", fg="black", font=("Helvetica", 25)) # def send_msg
mybutton8 = Button(root, text="STOP", command=buttonPush8, bg="red", fg="black", font=("Helvetica", 25)) #def stop

#Place buttons on grid
mybutton_window = my_canvas.create_window(10, 10, anchor='nw', window=mybutton)
mybutton2_window = my_canvas.create_window(10, 290, anchor='nw', window=mybutton2)
mybutton3_window = my_canvas.create_window(10, 360, anchor='nw', window=mybutton3)
mybutton4_window = my_canvas.create_window(10, 150, anchor='nw', window=mybutton4)
mybutton5_window = my_canvas.create_window(10, 220, anchor='nw', window=mybutton5)
mybutton6_window = my_canvas.create_window(10, 80, anchor='nw', window=mybutton6)
mybutton7_window = my_canvas.create_window(10, 430, anchor='nw', window=mybutton7)
mybutton8_window = my_canvas.create_window(10, 500, anchor='nw', window=mybutton8)

#configure grid to button
root.grid_rowconfigure(0, weight=1)
root.grid_columnconfigure(0, weight=1)
#---------------------------------END BUTTON SET UP
#implementing multithreading 
my_thread = threading.Thread(target=socketThread) # Create the thread
my_thread.start() # Start the thread


while 1:
    #global flag_updateGraph
    root.update()
    time.sleep(.1)
    if flag_updateGraph == 1:
        update_plot()
        flag_updateGraph = 0


# root.mainloop()
