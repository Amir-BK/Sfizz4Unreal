# Sfizz4Unreal
A small plugin that allows using Sfizz as a Synthesizer within Unreal and Metasounds, deisgned for Unreal 5.4+ with Harmonix plugin


https://github.com/user-attachments/assets/0fb548b0-e9be-42e1-931d-714dac7ac1ea



https://github.com/user-attachments/assets/c721e7af-2952-4a32-88f5-148737a2ca75



Uses and depends on Sfizz - https://github.com/sfztools/sfizz

The above demo videos use the wonderful and free Virtual Playing Orchestra libraries - http://virtualplaying.com/virtual-playing-orchestra/

Sound comparison between the default Fusion Piano and Mats Helgesson's Maestro Concert Grand Piano - https://github.com/sfzinstruments/MatsHelgesson.MaestroConcertGrandPiano/


https://github.com/user-attachments/assets/5d808e8f-4fd1-4b90-854e-76f2c9ece9c3



Recommendations:
1. Performance seems much better if the samples are on an SSD drive.
2. If attempting to use in an actual packaged game the paths provided to sfizz should be the correct absolute paths for the target machine
3. Sfizz can take a few seconds to load a large library, when used to render midi it's better to load play the metasound first and to give Sfizz a chance to load before sending play on the midi transport.

# Community/Feedback/Support -  
Please join the discord server - https://discord.gg/hTKjSfcbEn

