# tinyML-gesture-recognition
Proyect meant for the recognition of three different motions or gestures using tinyML (tiny Machine Learning) with the STM32f429idiscovery and its gyro.  Laboratory work from the course _Laboratory of Microcontrollers_ at University of Costa Rica .

### To execute:
* #### spi-gyro for gesture recording:
  * Clone the repository _libopencm3-examples_
  * Copy the folder /spi-gyro has to be copied into the folder _libopencm3-examples_ as: _libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/spi-gyro/_ and move to this location
  * Run 'make', then run './quemar.sh' on the terminal.
  
* #### STM32detection for the gesture recognition program:
  * Clone the repository MicroML and perform the Installation and setup indicated in the README.
  * Copy the folder /STM32detection into the _MicroML_ folder as: /MicroML/projects/STM32detection/ and move to this folder.
  * Run 'make', then 'cd build' and then './quemar.sh' on the terminal
