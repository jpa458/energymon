energymon
=========

The software behind my DIY energy monitoring project.

Hardware
--------
My setup is based on components from [JeeLabs](http://jeelabs.org)
* JeeLink central node for receiving all remote sensor output
* JeeNode and LuxPlug to count and broadcast LED pulses from my electricity meter

Sofware
-------
* Logger : Logs incoming traffic from remote sensors to a file.
* PulseReader : Sketch to count LED pulses and broadcast the result. 

See file headers for more details.