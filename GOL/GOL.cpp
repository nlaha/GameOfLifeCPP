// GOL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <chrono>

#include "Board.h"

/* MAIN */
int main()
{
    const uint8_t BOARDSIZE_X = 20; // Board width
    const uint8_t BOARDSIZE_Y = 20; // Board height

    Board mainBoard(BOARDSIZE_X, BOARDSIZE_Y);

    // Load the txt file
    mainBoard.loadFromFile("Board.txt");

    // Print the generations to the console
    while (true) {

        // clear the screen
        system("CLS");
            
        mainBoard.show();
        mainBoard.nextGeneration();

        system("pause"); // Wait for user input to step through each generation
    }
}