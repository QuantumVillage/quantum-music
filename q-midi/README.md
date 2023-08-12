# Quantum MIDI

This is a very simple example that performs two things using the [micro-qsim](https://github.com/Quantum-Village/micro-quantum) library QV released in 2022. 

This will build in the usual way with the `pico-sdk` for the Rasspberry Pi Pico:

```bash
mkdir build
cmake ..
make
```

This particular version has several interrupts. The interrupt for the button will generate a simulation with lots of outputs that become the input parameters for the oscillators in the DX. The UART interrupt allows notes to be read , pass the note value to a quantum simulator, and then read the response and send it as a new MIDI note. 

MIDI is a very simple protocol (3 byte messages at 31.25k speed, 8N1), so it is implemented manually here.