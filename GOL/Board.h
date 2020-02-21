#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

class Board
{
    private:
        /**
        * Checks the cell surroundings for live cells and returns the number
        *
        * @param board is a 2D vector of characters with the input generation
        * @param row the row to check
        * @param col the column to check
        */
        int checkNeighbors(std::vector<std::vector<char>> board, int row, int col) {
            // start checking for neighbors
            int numNeighbors = 0;
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    // Check to make sure we're not out of bounds
                    if ((row + i) < BOARDSIZE_X && (col + j) < BOARDSIZE_Y) {
                        if ((row + i) > 0 && (col + j) > 0) {
                            // Check to make sure we're not on the cell itself
                            if (!(i == 0 && j == 0)) {
                                // Check to make sure the neighbor is alive
                                if (board[row + i][col + j] == '#') {
                                    numNeighbors++; // Add a neighbor
                                }
                            }
                        }
                    }
                }
            }
            return numNeighbors;
        }

        uint8_t BOARDSIZE_Y = 20;
        uint8_t BOARDSIZE_X = 20;
        uint16_t generation = 0;
        std::vector<std::vector<char>> board; // vector that stores the board

    public:

        /* CONSTRUCTOR */
        Board(uint8_t BOARDSIZE_X, uint8_t BOARDSIZE_Y) {
            board.resize(BOARDSIZE_Y, std::vector<char>(BOARDSIZE_X));
        }

        /**
        * Returns a 2D vector of characters with the next generation
        *
        * @param board is a 2D vector of characters with the input generation
        */
        void nextGeneration() {
            std::vector<std::vector<char>> boardTemp(BOARDSIZE_Y, std::vector<char>(BOARDSIZE_X));

            for (int row = 0; row < BOARDSIZE_Y; row++) // loop through rows
            {
                for (int col = 0; col < BOARDSIZE_X; col++) // loop through columns
                {
                    int numNeighbors = 0;
                    //std::future<int> fNeighbors = std::async(std::launch::async, &checkNeighbors, board, row, col, BOARDSIZE_X, BOARDSIZE_Y);
                    //numNeighbors = fNeighbors.get();
                    numNeighbors = checkNeighbors(board, row, col);

                    /*  GOL RULES BELOW  */

                    // Cell dies (1 or 0 neighbors)
                    if (numNeighbors < 2) {
                        boardTemp[row][col] = '.';
                    }
                    // Cell dies (> 3 neighbors)
                    else if (numNeighbors > 3) {
                        boardTemp[row][col] = '.';
                    }
                    // Call grows (exactly 3 neighbors)
                    else if (numNeighbors == 3) {
                        boardTemp[row][col] = '#';
                    }
                    // if it's not one of these things then just do nothing
                    else {
                        boardTemp[row][col] = board[row][col];
                    }
                }
            }

            board = boardTemp;
        }

        /**
        * Loads the board form a text file
        * The file should contain "." for dead cells and "#" for alive cells
        * @param filename is the name of the file to read
        */
        void loadFromFile(std::string filename) {
            std::ifstream infile(filename); // .txt file containing board "." as dead and "#" as alive

            // Variables for reading file
            int frow = 0;
            int fcol = 0;
            char c;
            while (infile.get(c)) // read file
            {
                if (c != *"\n") { // handle newlines
                    board[frow][fcol] = c;
                    fcol++;
                }
                else { // reset columns and add a row
                    frow++;
                    fcol = 0;
                }
            }

        }

        // Prints the board to the console
        void show() {

            // Get Start Time
            std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

            std::cout << "Generation: " + std::to_string(generation) << std::endl;

            for (int i = 0; i < BOARDSIZE_Y; i++)
            {
                for (int j = 0; j < BOARDSIZE_X; j++)
                {
                    std::cout << board[i][j] << ' ';
                }
                std::cout << std::endl;
            }
            std::cout << '\n';

            // Get End Time
            auto end = std::chrono::system_clock::now();

            auto diff = std::chrono::duration_cast <std::chrono::milliseconds> (end - start).count();
            std::cout << "Time to compute = " << diff << " Milliseconds" << std::endl;

            generation++;
        }
};

