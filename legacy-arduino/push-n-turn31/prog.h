/* programming documentation

start prog
./midiutil.py -d 1 -w 186 102 42

end prog
./midiutil.py -d 1 -w 186 102 43

module type
./midiutil.py -d 1 -w 186 102 44
returns: (10, 103, INSTRUMENT_TYPE);


firmware version
./midiutil.py -d 1 -w 186 102 45
returns (10, 104, FW_VERSION) 10 for version 1.0

show configuration
./midiutil.py -d 1 -w 186 102 46
returns the config of each of the 6 buttons, the rotary knob and rotary encoder
c
show stepsize 
./midiutil.py -d 1 -w 186 102 47
returns (10, 105, encStepSize)

show MIDI channel 
./midiutil.py -d 1 -w 186 102 48
returns (10, 106, MIDIChannel)

reboot in firmware update mode
./midiutil.py -d 1 -w 186 102 49

programming keys
./midiutil.py -d 1 -w 186 102 42
./midiutil.py -d 1 -w 186 11 60
./midiutil.py -d 1 -w 186 22 30
./midiutil.py -d 1 -w 186 33 9
./midiutil.py -d 1 -w 186 24 15
./midiutil.py -d 1 -w 186 5 24
./midiutil.py -d 1 -w 186 102 43


Voor Rutger's 4+1
./midiutil.py -d 1 -w 186 102 42
./midiutil.py -d 1 -w 186 31 2
./midiutil.py -d 1 -w 186 32 1
./midiutil.py -d 1 -w 186 33 64
./midiutil.py -d 1 -w 186 34 67
./midiutil.py -d 1 -w 186 35 10
./midiutil.py -d 1 -w 186 45 11
./midiutil.py -d 1 -w 186 50 15
./midiutil.py -d 1 -w 186 66 5
./midiutil.py -d 1 -w 186 102 43

line 1  // start prog
set switch 1 to CC momentary CC 2
set switch 2 to CC momentary CC 1
set switch 3 to CC momentary CC 64
set switch 4 to CC momentary CC 67
set encoder switch to CC 10
set encoder turner to CC 11
set midi channel to 16
set encoder steps to 5
end prog
*/