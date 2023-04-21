# BoomBox
Arduino mp3 player for children


## Prepare SD-Card

The <patches.053> file goes into the root folder. 

Music goes to folders called  
Music1  
Music2  
Music3  
Music4  


The files in the folders must be numbered track001.mp3 and so on. 
Currently this is necessary as using actual file names quickly used up Arduino SRAM. 
This may be fixable in the future by careful memory management and good software design.


## Button layout

1  2  3  
4  5  6  

1  pause  
2  cycle through folder Music4   
3  volume  
4  cycle through folder Music1  
5  cycle through folder Music2  
6  cycle through folder Music3  


## Playback mode (optional)

A file called <playbackMode.txt> with the following contents  

Music1 one  
Music2 all  
Music3 one  
Music4 all  

goes into the root folder.
It controls if the playlists stop after one song or continue playing.  
Default is to stop after each song.


## Tools

Electronics Layout: Frizing  
Software development: PlatformIO plugin for VisualStudio Code