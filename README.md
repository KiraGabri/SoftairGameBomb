# SoftairGameBomb
This project, developed as part of the **ARE 2022 university course**, aims to create a prototype of a functional replica of a bomb inspired by those seen in tactical shooter video games, designed for **airsoft “plant-the-bomb” scenarios**.

The system consists of two main components:  
1. **Detonator (Bomb)**  
2. **Disarm Device**

Through a setup menu, the game organizer can define a **minimum countdown time** for detonation. The player planting the bomb must enter a **disarm password** and a **valid countdown** (minutes and seconds) that meets or exceeds the minimum limit.

Players can disarm the bomb in two ways:  
- By entering the **correct password**  
- By using the **physical disarm device**, which must remain connected for a period proportional to the remaining countdown time

If an incorrect password is entered, the buzzer emits an error sound. Disconnecting the disarm device during the process resets the disarm sequence.

The circuit was first designed and simulated using **Tinkercad** before physical implementation.


**Technologies:** Arduino, C++, electronics (buzzer, keypad, display, timer logic).

## License / Use
This project is released **for demonstration purposes for portfolio use only**.   
Redistribution, modification, or use of the source code without permission is not allowed.
