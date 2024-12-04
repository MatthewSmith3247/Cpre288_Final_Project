from tkinter import *
import tkinter

#USE PIP INSTALL in the Terminal line

#Had to install SpeechRecognition and pyaudio for listener
import speech_recognition as sr 

#for speaker install pyttsx3
import pyttsx3

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
global word_said, text_to_speak
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
    engine = pyttsx3.init()
    #set the rate it talks in words per minute
    engine.setProperty('rate', 150)
    voices = engine.getProperty('voices')
    #change voice to index 1 for female adn 0 for male
    engine.setProperty('voice', voices[1].id)
    engine.setProperty('volume', 3.0)
    engine.say(text_to_speak)
    engine.runAndWait()

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
        print("Received line:", line)  # Debug print
        clean_line = re.sub(r'[\x00]', '', line)  # Remove specific unwanted characters
        data = clean_line.split() # splits the line based on white space
        print("Received data:", data)  # Debug print
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
    ax.set_rticks([0.5, 1, 1.5, 2, 2.5])   # Set plot "distance" tick marks at .5, 1, 1.5, 2, and 2.5 meters
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
    stairwell = patches.Rectangle((1.835,0.895), width=.61, height=.61, linewidth=4, edgecolor='black', facecolor='black')
    ax.add_patch(stairwell)

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
    ax.scatter(x_values, y_values, color='blue', label='Data Points', s=100)
    #plot Short objects
    ax.scatter(x_values_short, y_values_short, color='red', label='Data Points2', s=100)
    #plot cybot depending on direction it is facing
    match direction:
        case "north":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='v', s=125)
        case "south":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='^', s=125)
        case "east":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='<', s=125)
        case "west":
            ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='>', s=125)
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
    send_message = input("Enter a message (enter quit to exit):") + '\n' # Enter next message to send to server
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
        "west" : (0.5, 0),
        "east" : (-0.5, 0),
        "south" : (0, 0.5)
    }
    global cybotx, cyboty, direction
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
    update_plot()
        
#--------------------------------- Basic Driving functions
def forward():
    send_message = "w\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Got a message from server: " + clean_line + "\n") # Convert message from bytes to String (i.e., decode)
    update_plot()   

def turn_right():
    send_message = "d\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    update_plot()   

def turn_left():
    send_message = "a\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server 
    update_plot() 

def backward():
    send_message = "s\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server 
    update_plot()   

def stop():
    send_message = "q\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Got a message from server: " + clean_line + "\n") # Convert message from bytes to String (i.e., decode)
    update_plot()   

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

def takingScanData():
    line = cybot.readline().decode().strip() 
    count = 0
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
            distance = float(data[0])/1000
            angle = int(data[1])
            count += 1
            print("Distance: " + str(distance) + " Angle: " + str(angle))
            if distance < 50 and angle > 90 and angle < 170:
                #Use those values to find the coordinates of the new object
                angle -= 90
                x_coord = cybotx + (distance*np.cos(np.deg2rad(angle)))
                y_coord = cyboty + (distance*np.sin(np.deg2rad(angle)))
                print("X Coord: " + str(x_coord) + " Y Coord: " + str(y_coord))
                #append to array
                x_values = np.append(x_values, x_coord)
                y_values = np.append(y_values, y_coord)
            elif distance < 50 and angle >= 20 and angle <= 90:
                #Use those values to find the coordinates of the new object
                angle = 90 - angle
                x_coord = cybotx+ (distance*np.cos(np.deg2rad(angle)))
                y_coord = cyboty + (distance*np.sin(np.deg2rad(angle)))
                print("X Coord: " + str(x_coord) + " Y Coord: " + str(y_coord))
                #append to array
                x_values = np.append(x_values, x_coord)
                y_values = np.append(y_values, y_coord)
        finally:
            line = cybot.readline().decode().strip() 
            # Remove specific unwanted characters  
            clean_line = re.sub(r'[\x00]', '', line)
    print("count: " + str(count))

def setLoc():
    global cybotx, cyboty
    cybotx = 3.75
    cyboty = .57
    update_plot()
#--------------------------------- Cybot Tracking Functions     
def bumpLeft():
    #the cybot hit an object with the left bump sensor 
    global  cybotx, cyboty, x_values_short, y_values_short
    #Need to recieve the distance traveled from the cybot in order to pinpoint object location ONLY NEED X DIRECTION
    #line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    #clean_line = re.sub(r'[\x00]', '', line) 
    #Object is right to the left of the cybot 
    #add values to the array
    x_values_short = np.append(x_values_short, cybotx)
    y_values_short = np.append(y_values_short, cyboty)
    #update cybot location // only y direction though??? FIXME
    update_plot()
    #Now cybot will scan to the right to see if it is an ideal move path???
    # FIXME 

def bumpRight():
    #the cybot hit an object with the left bump sensor 
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
    update_plot()
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
    # this function should move the bot from the bottom left to the bottom right 
    send_message = "1\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    time.sleep(0.5)
    #print("sent message: " + send_message) 
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    #print("Received message " + clean_line + "\n") 
    progress = 'no'
    movement = {
        "north" : (0, -0.5),
        "west" : (0.5, 0),
        "east" : (-0.5, 0),
        "south" : (0, 0.5)
    }
    while 'done' not in progress:
        global cybotx, cyboty, direction
        #print("In Loop Received message " + clean_line + "\n")
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
            takingScanData()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            update_plot()
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
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        if cybotx >= 3.65:
            if cyboty <= .61 and 'west' in direction:
                progress = 'done'
                send_message = "1\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            elif cyboty <= .61 and 'west' not in direction:
                match direction:
                    case "north":
                        #turn 1 CCW
                        send_message = "left\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "south":
                        #turn 1 CW
                        send_message = "right\n"  
                        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                    case "east":
                        #turn 2 CW
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
        print("X: " + str(cybotx) + " Y: " + str(cyboty) + "Dir: " + direction)
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
    #update_plot()
    send_message = "1\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server 

def find_kitchen_step1():
    #Move across near the bedroom
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    progress = 'no'
    while 'done' not in progress:
        global cybotx, cyboty, direction
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
            takingScanData()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            update_plot()
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
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        if cybotx <= 0.90:
            if 'east' in direction:
                send_message = "1\n"  
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
        update_plot()
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
    send_message = "1\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

def find_kitchen_full():
    # this function should move the bot from the bottom right to the top left 
    send_message = "2\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    time.sleep(0.5)
    #I split it into 2 steps to make the code more readable, step 1 is a different function
    find_kitchen_step1()
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    progress = 'no'
    while 'done' not in progress:
        global cybotx, cyboty, direction
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
            takingScanData()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            update_plot()
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
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
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
                send_message = "1\n"  
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
        update_plot()
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
    send_message = "1\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

def find_livingRoom():
     # this function should move the bot from the top left to the top right 
    send_message = "3\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
    time.sleep(0.5)
    print("sent message: " + send_message) 
    line = cybot.readline().decode().strip() 
     # Remove specific unwanted characters  
    clean_line = re.sub(r'[\x00]', '', line)
    print("Received message " + clean_line + "\n") 
    progress = 'no'
    while 'done' not in progress:
        global cybotx, cyboty, direction
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
            takingScanData()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            update_plot()
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
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        if cybotx >= 3.62:
            if cyboty >= 1.75 and 'west' in direction:
                progress = 'done'
                send_message = "1\n"  
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            #FIXME Finish this
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
    send_message = "1\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

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
        global cybotx, cyboty, direction
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
            takingScanData()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'counterclockwise' in clean_line:
            turnedCounterClockwise()
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        elif 'clockwise' in clean_line:
            turnedClockwise()
            update_plot()
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
            update_plot()
            send_message = "0\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        if cyboty >= 1.0 and cyboty <= 1.75:
            progress = 'done'
            send_message = "1\n"  
            cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
            #FIXME Finish this
        line = cybot.readline().decode().strip() 
        # Remove specific unwanted characters  
        clean_line = re.sub(r'[\x00]', '', line)
    send_message = "1\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

#creating buttons Widget
mybutton = Button(root, text="Listen", command=listen_for_command, bg="red", fg="black", font=("Helvetica", 25)) 
mybutton2 = Button(root, text="To Living Room", command=find_livingRoom, bg="red", fg="black", font=("Helvetica", 25)) 
mybutton3 = Button(root, text="To Exit", command=find_exit, bg="red", fg="black", font=("Helvetica", 25)) 
mybutton4 = Button(root, text="To Bathroom", command=find_bathroom, bg="red", fg="black", font=("Helvetica", 25)) 
mybutton5 = Button(root, text="To Kitchen", command=find_kitchen_full, bg="red", fg="black", font=("Helvetica", 25)) 
mybutton6 = Button(root, text="Responder", command=respond, bg="red", fg="black", font=("Helvetica", 25)) 
mybutton7 = Button(root, text="Send Msg", command=send_msg, bg="red", fg="black", font=("Helvetica", 25)) 
mybutton8 = Button(root, text="STOP", command=stop, bg="red", fg="black", font=("Helvetica", 25)) 

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

root.mainloop()
