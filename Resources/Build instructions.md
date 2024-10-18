# Making
## Display
The display I ordered came with a ribbon cable. I cut the ends off and soldered it directly to the corresponding holes in the PCB. 

## LED arrays
The LED arrays are made by bending the 1.5mm solid core wire into shape and soldering the LED bars to them. I 3D printed a guide to help with correctly bending and cutting the wire to length. The LED bars require quite some heat to solder. I found it easier to solder them onto the wire first, then cutting it to length and bending them.

## RTC module
I made sure to solder the headers in such a way that the battery holder on the RTC module is facing away from the PCB. 

# Music upload
The BY8301-16P is a finicky board. I still not seem to get consistent audio from it. See this GitHub for more information:
https://github.com/NachtRaveVL/BY8X01-16P-Arduino

To upload music:

1. Connect through micro USB
2. A CD-drive folder should appear. Double click to launch.
3. In the screen that opens, go to the second tab.
4. Click on the button, then select the music files you want to upload (be mindful of the limited flash available)
5. Go to the first tab.
6. Click on the button. If you see something similar to "#@%(Flash..." it is uploading the files. If you see something similar to "#%%Flash!" your files are probably too large to upload. 
