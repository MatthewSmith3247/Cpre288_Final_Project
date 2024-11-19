
#pip install matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as patches

import numpy as np

# Create a figure and axes
fig, ax = plt.subplots()

# Example data: x and y values in arrays
x_values = np.array([1, 2, 3, 4, 5])  # x coordinates of points
y_values = np.array([5, 4, 3, 2, 1])  # y coordinates of points

#Cybot
global cybotx, cyboty, theta, distance, angle
cybotx= 2
cyboty = 1
theta = 0


#Adding 'rooms'
def plot_rooms():
    #Bedroom
    bedroom = patches.Rectangle((0,0), width=3, height=3, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(bedroom)
    #Bathroom
    bathroom = patches.Rectangle((7,0), width=3, height=3, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(bathroom)
    #Living Room
    living_room = patches.Rectangle((7,7), width=3, height=3, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(living_room)
    #Kitchen
    kitchen = patches.Rectangle((0,7), width=3, height=3, linewidth=4, edgecolor='purple', facecolor='none')
    ax.add_patch(kitchen)
    #Stairwell
    stairwell = patches.Rectangle((4.25,4.25), width=1.5, height=1.5, linewidth=4, edgecolor='black', facecolor='black')
    ax.add_patch(stairwell)

def plot_config():

    #Set Size of Plot
    ax.set_xlim(0,10)
    ax.set_ylim(0,10)

    #Arranging Grid Size
    x_gridlines = np.arange(0,11,1)
    y_gridlines = np.arange(0,11,1)
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
#plot points
ax.scatter(x_values, y_values, color='blue', label='Data Points')
#plot cybot
ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='s', s=100)

#interactive mode to update plot dynamically
plt.ion()

#display the plot
plt.show()

def update_plot():
    global cybotx
    global cyboty
    #clear current plot
    ax.clear() 
    #update cybot values
    cybotx += 1 
    #plot points
    ax.scatter(x_values, y_values, color='blue', label='Data Points')
    #plot cybot
    ax.scatter(cybotx, cyboty, color='green', label='Cybot', marker='s', s=100)
    #add 'rooms'
    plot_rooms()
    #add config
    plot_config()
    #redraw plot
    plt.draw()

def add_object():
    #hopefully adding any object that the cybot "sees"
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
    x_values = np.append(x_values, x_input)
    y_values = np.append(y_values, y_input)


while True:
    #get user input
    user_input = input("Enter a new point (x,y): ")

    #split values
    x_input, y_input = map(float, user_input.strip("()").split(","))

    #add values to the array
    x_values = np.append(x_values, x_input)
    y_values = np.append(y_values, y_input)

    #update plot
    update_plot()

