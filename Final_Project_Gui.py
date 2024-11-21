from tkinter import *
import tkinter

#USE PIP INSTALL in the Terminal line

#Had to install SpeechRecognition and pyaudio
import speech_recognition as sr 

#For Socket Connection
import time
import socket
# See: Background, Socket API Overview, and TCP Sockets    
import re

#Import/Include useful math and plotting functions
import numpy as np
import matplotlib.pyplot as plt
import os  # import function for finding absolute path to this python script

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
global word_said
word_said = ""

#---------------------------------Listener
def listen_for_command():
    recognizer = sr.Recognizer()
    global word_said
    with sr.Microphone() as source:
        print("Listening...")
        audio = recognizer.listen(source)
        print("Not Listening")
    
    try: 
        word_said = recognizer.recognize_google(audio)
        #cybot.write(word_said.encode()) # Convert String to bytes (i.e., encode), and send data to the server
        print("Text: "+ word_said)
        
    except:
        print("Sorry, I did not get that")
        word_said = "I don't know"

    if 'Find me':
        #Send message to Cybot
        word_said = "Something"
    


def respond():
    global word_said
    if 'hi ' in word_said:
        print("Hi to you too!")
        #mylabel = Label(root, text="Hi to you too!")
        #mylabel.pack()
    else:
        print("I don't know that one")
        #mylabel = Label(root, text="I don't know that one")
        #mylabel.pack()

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
    ax.grid(True)                     # Show grid lines

# Create title for plot (font size = 14pt, y & pad controls title vertical location)
    ax.set_title("Mock-up Polar Plot of CyBot Sensor Scan from 0 to 180 Degrees", size=14, y=1.0, pad=-24) 
    plt.show()  # Display plot
#---------------------------------End of Plot

#---------------------------------Drive
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
        

def forward():
    send_message = "w\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server      

def stop():
    send_message = "q\n"  
    cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server   

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
        

#creating buttons Widget
mybutton = Button(root, text="Listen", command=listen_for_command, bg="red", fg="black", font=("Helvetica", 25)) # def on line 64
mybutton2 = Button(root, text="Send Msg", command=send_msg, bg="red", fg="black", font=("Helvetica", 25)) # def on line 144
mybutton3 = Button(root, text="Drive Fwd", command=forward, bg="red", fg="black", font=("Helvetica", 25)) # def on line 158
mybutton4 = Button(root, text="Stop", command=stop, bg="red", fg="black", font=("Helvetica", 25)) # def on line 162
mybutton5 = Button(root, text="Scan", command=scan, bg="red", fg="black", font=("Helvetica", 25)) # def on line 166
mybutton6 = Button(root, text="Responder", command=respond, bg="red", fg="black", font=("Helvetica", 25)) # def on line 86

#Place buttons on grid
mybutton_window = my_canvas.create_window(10, 10, anchor='nw', window=mybutton)
mybutton2_window = my_canvas.create_window(10, 360, anchor='nw', window=mybutton2)
mybutton3_window = my_canvas.create_window(10, 150, anchor='nw', window=mybutton3)
mybutton4_window = my_canvas.create_window(10, 220, anchor='nw', window=mybutton4)
mybutton5_window = my_canvas.create_window(10, 290, anchor='nw', window=mybutton5)
mybutton6_window = my_canvas.create_window(10, 80, anchor='nw', window=mybutton6)

#configure grid to button
root.grid_rowconfigure(0, weight=1)
root.grid_columnconfigure(0, weight=1)
#---------------------------------END BUTTON SET UP


root.mainloop()