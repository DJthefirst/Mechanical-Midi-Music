# Overview
This is a continuation of my Musial Instument project you Reviewed about a year ago. Since then I have worked to refine both the program and my skills based upon your previous feedback. The code works and there are no major bugs that I am aware of. The main intent of this review is to identify sylistic problems and potential optomizations.

### Context
Link to a video using this project
https://www.youtube.com/watch?v=XZTp6t2umNo

The GUI is entirely webbased using webserial to connect to each microcontroller. I can show you it on the call but it is not within the scope of this review https://mechanical-midi-music.vercel.app/

### Setup
The code can be found at https://github.com/DJthefirst/Mechanical-Midi-Music the Main branch.
You should only need the PlatformIO extension and open the ESP32_Client directory containing the __platformio.ini__ file

### Scope
The configuration should be setup already with the Led Extras dissabled and in __platformio.ini__ check_flags = supessing unusedFunction warnings. 

- In __Networks/__ ignore __NetworkDIN__ and __NetworkUDP__. 
- In __Instuments/Default__ only look at __PwmDriver__
- Ignore __Instuments/DJthefirst__
- In __Configs/__ only look at __ExampleConfig__

### General
(Low) I am planning to make a version of this code for leagcy microcontrolles not ESP32 running c++ 11. If you notice any C++ 17 features I am using other than in Instrument __Insturments/InterruptTimer.h__ and __Device.h__

# Specific issues
### Main

I think main is overall pretty good.

1. (Medium) The instument controller and connection type are set by defines from the Config file. I would like to somehow pass the class type as const instead. My attempt at this is __Instruments/Instrument.h__ where a new Instument controller could be constructed from a template
``` c++
template<InstrumentType INSTRUMENT_TYPE> 
```
But I was not exactly sure how to implement this sytle over the current virtual functions. It was my effort to think up some design that does not rely on virtual functions at all. Only one Instrument Constroller or Network exists at any given time so I am only wasting preformance and Memory.

2. (Medium) Message handler and Network require pointers to eachother. To do this the pointers are set by functions leading them not to be defined in the constructor. I do not want one class containing the other or a super class that mediates comunication between them. The current method works but I did not know if there was a more perfesional/ safe solution.
>[medium:warning] Member variable 'Network::m_ptrMessageHandler' is not initialized in the constructor. Maybe it should be initialized directly in the class Network? [uninitDerivedMemberVar]

> [medium:warning] Member variable 'MessageHandler::m_ptrNetwork' is not initialized in the constructor. [uninitMemberVar]

3. (Low) I would like to move the #ifdef LOCAL_STORAGE code into __Extras/LocalStorage__ but I would need to add dependencies to LocalStorage or pass everything into it. I can probaly fix this myself just listing it for sake of documentation.

### Constants

1. (Low) I put most of my Enums in conatants but I am torn wether to keep them there or move them into their respective classes __Instruments__, __Device__, and __Distributor__. I belive I ran into cyclical dependencie problems which is why I moved them in the first place.

### Configs/ExampleConfig

1. (Medium) Discussed above replace the Defines for PwmDriver and NetworkUSB. I think the Defines for LOCAL_STORAGE and ADDRESSABLE_LEDS are ok.

### Device
1. (High) Would name being a static char [20] improve effciency? I tried to do this but ran into problems when trying to rename it. Also how would both inline variable be rewritten for c++ 11 for legacy support.

1. (Medium) Device is a namespace. Almost every class uses Device and their is only one instance of it. I think this is the correct choice but just making sure.

2. (Low) You select your config by including it and then other files only include Device.h. I am not sure how standard this is or if there is a better way.

3. (Low) The default config is set by the constants in this file. The PLATFORM_ Defines are generated by Platform IO. Currently the Pin config comes from Device.h but I would like a way to create a constant in __Config__ that overwrites the default.

### Message Handler

1. (Low) Thoughts on how I implemeted Local Storage at the bottom of the function.

### Extras/LocalStorage

1. (Medium) This part of the code was an addon or "__Extra__" which involves a lot of pointer manuipulation. It allows the microcontroller to save its configuration when power is lost. I am not entirely sure I did everything correctly. I probably should use smart pointers somewhere?
