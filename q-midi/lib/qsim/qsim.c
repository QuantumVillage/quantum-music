#include "qsim.h"
#include "gates.h"
#include "measure.h"
#include "img_hex.h"
#include <string.h>
#include <math.h>
#include <stdint.h>

uint8_t num_slices = 6;
uint8_t num_qubits = 2;

char bin_strs[4][5] = {
    "00",
    "01",
    "10",
    "11",
};


void init_circuit(char circuit[num_slices][num_qubits][8])
{
    for (int i = 0; i < num_slices; i++)
    {
        for (int j = 0; j < num_qubits; j++)
        {
            strcpy(circuit[i][j], "-");
        }
    }
}



int qsim(uint8_t val)
{
    double complex stateVec[4] = {0};
    stateVec[0] = 1 + 0 * I;
    char circuit[num_slices][num_qubits][8];

    // init circuit
    init_circuit(circuit);

    // setup entanglement circuit
    //strcpy(circuit[0][0], "H");
    //strcpy(circuit[1][0], "c");
    //strcpy(circuit[1][1], "X");

    // setup entanglement circuit
    strcpy(circuit[0][0], "H");
    strcpy(circuit[0][1], "R");
    strcpy(circuit[1][0], "c");
    strcpy(circuit[1][1], "X");


    // do measurement! 
    measure(circuit, stateVec);
    //draw_results(stateVec, BlackImage);



}
