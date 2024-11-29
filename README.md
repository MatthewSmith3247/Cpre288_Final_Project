CprE 288 Embedded Systems Final Project: RescueBot (Fall 2024)

Team: Rescue Robotics
Matthew Smith: mss3247@iastate.edu
Jessica Reiff: jreiff@iastate.edu
Luke Bronzy: lukebro@iastate.edu
Carson Stanley: stanley9@iastate.edu

Story
Briefing: RescueBot is an innovative assistive robot designed to help visually impaired individuals navigate new and cluttered indoor environments. Inspired by the functionality of a service animal, RescueBot offers a cost-effective and accessible alternative. Using a combination of IR and Ping sensors, audio feedback, and voice command capabilities, RescueBot provides users with confidence and independence in their mobility.

Our project leverages the capabilities of the iRobot Create platform, integrating custom software, hardware, and sensors to deliver real-time navigation assistance. RescueBot is designed to detect and avoid obstacles, guide users safely, and provide intuitive voice-based interaction.

Problem Statement
For visually impaired individuals, navigating new environments can be challenging and often requires service animals or caregivers—resources that are either expensive or inaccessible to many. RescueBot bridges this gap by providing a reliable and affordable alternative, enabling users to move independently while receiving constant audio feedback about their surroundings.

Functional Requirements
RescueBot incorporates the following features:

Obstacle Detection and Navigation
Scans the environment using IR and Ping sensors mounted on a 180-degree servo.
Detects and avoids obstacles, rerouting when necessary to ensure a safe path.
Audio Feedback
Provides clear, friendly voice feedback about detected objects and navigation updates.
Adjustable audio output to suit different environments.
Voice Commands
GUI-based interface to accept and execute user commands for destination and mode selection.
Allows intuitive interaction for users with limited vision.
Pathfinding
Implements algorithms to optimize navigation and ensure safe traversal in cluttered environments.
Push Small Objects
Mechanically moves small obstacles out of the user’s path.
Project Description
RescueBot is a navigation assistant designed for dense indoor environments, offering a safe and efficient alternative to traditional mobility aids. Key tasks include:

Pathfinding & Navigation: Ensuring safe travel through cluttered spaces using advanced sensors and servo-mounted scanning systems.
User Interaction: Simplified voice commands via a GUI interface and clear audio output to guide users effectively.
Adaptive Mobility: Automatically reroutes to avoid obstacles, ensuring the user can navigate confidently and safely.
Technical Highlights
Hardware:
iRobot Create platform.
IR and Ping sensors for object detection.
180-degree servo for obstacle scanning.
Integrated audio system for output and feedback.
Software:
Dijkstra’s algorithm for optimized pathfinding.
GUI for voice-based user input and feedback.
Modular codebase for movement, obstacle detection, and feedback systems.
Team Contributions
Matthew Smith: Developed the audio output system, integrating custom voice prompts and hardware communication for seamless interaction.
Luke Bronzy: Focused on object detection, movement algorithms, and system integration for efficient navigation.
Carson Stanley: Wrote autonomous navigation code, optimizing sensor data processing for real-time rerouting.
Jessica Reiff: Designed and implemented the GUI for user interaction, enabling voice command input and device feedback.
Future Applications
RescueBot serves as a stepping stone for affordable assistive technology. It has potential for further development, including outdoor navigation, enhanced obstacle detection, and advanced voice interaction systems.
