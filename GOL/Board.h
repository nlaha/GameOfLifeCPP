#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>    // std::for_each
#include <string>

class Board
{
    private:

        uint16_t BOARDSIZE_Y = 20;
        uint16_t BOARDSIZE_X = 20;
        std::vector<std::vector<char>> board; // vector that stores the board
        std::vector<std::vector<int>> boardAge; // vector that stores the age of each cell

    public:

        uint16_t generation = 0; // making this public cause a getgeneration() function would be slow

        /* CONSTRUCTOR */
        Board(uint16_t newBOARDSIZE_X, uint16_t newBOARDSIZE_Y) {
            BOARDSIZE_X = newBOARDSIZE_X;
            BOARDSIZE_Y = newBOARDSIZE_Y;

            boardAge.resize(BOARDSIZE_Y, std::vector<int>(BOARDSIZE_X));

            board.resize(BOARDSIZE_Y, std::vector<char>(BOARDSIZE_X));
        }

        /**
        * Returns a 2D vector of characters with the next generation
        *
        * @param board is a 2D vector of characters with the input generation
        */
        void nextGeneration() {

            generation++; // increment the generation
            std::vector<std::vector<char>> boardTemp(BOARDSIZE_Y, std::vector<char>(BOARDSIZE_X));

            tbb::parallel_for(tbb::blocked_range<uint16_t>(0, BOARDSIZE_Y), [&](tbb::blocked_range<uint16_t> ib)
            {
                tbb::parallel_for(tbb::blocked_range<uint16_t>(0, BOARDSIZE_X), [&](tbb::blocked_range<uint16_t> jb)
                {
                    // These loops are divided up across all threads
                    for (int row = ib.begin(); row < ib.end(); ++row)
                    {
                        for (int col = jb.begin(); col < jb.end(); ++col)
                        {
                            int numNeighbors = 0;

                            // Check for new neighbors
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
                                                if (getCell(row + i, col + j) == '#') {
                                                    numNeighbors++; // Add a neighbor
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            /*  GOL RULES BELOW  */

                            // Cell dies (1 or 0 neighbors)
                            if (numNeighbors < 2) {
                                boardTemp[row][col] = '.';
                                setCellAge(row, col, 0);
                            }
                            // Cell dies (> 3 neighbors)
                            else if (numNeighbors > 3) {
                                boardTemp[row][col] = '.';
                                setCellAge(row, col, 0);
                            }
                            // Call grows (exactly 3 neighbors)
                            else if (numNeighbors == 3 && getCell(row, col) == '.') {
                                boardTemp[row][col] = '#';
                                setCellAge(row, col, 0);
                            }
                            // if it's not one of these things then just up the age counter
                            else {
                                boardTemp[row][col] = getCell(row, col);
                                int upAge = getCellAge(row, col);
                                upAge++;
                                setCellAge(row, col, upAge);
                            }
                        }
                    }
                });
            });

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

            std::cout << "Generation: " + std::to_string(generation) << '\n';

            for (int i = 0; i < BOARDSIZE_Y; i++)
            {
                for (int j = 0; j < BOARDSIZE_X; j++)
                {
                    std::cout << board[i][j] << ' ';
                }
                std::cout << '\n';
            }
            std::cout << '\n';

        }

        std::vector<std::vector<char>> getBoard() {
            return board;
        }

        std::vector<std::vector<int>> getBoardAge() {
            return boardAge;
        }

        int getCellAge(int row, int col) {
            return boardAge[row][col];
        }

        char getCell(int row, int col) {
            return board[row][col];
        }

        void setBoard(std::vector<std::vector<char>> newBoard) {
            board = newBoard;
        }

        void setCell(int row, int col, char state) {
            if (row < BOARDSIZE_X && col < BOARDSIZE_Y) {
                if (row > 0 && col > 0) {
                    board[row][col] = state;
                }
            }
        }

        void setCellAge(int row, int col, int age) {
            if (row < BOARDSIZE_X && col < BOARDSIZE_Y) {
                if (row > 0 && col > 0) {
                    boardAge[row][col] = age;
                }
            }
        }

        void clearBoard() {
            std::vector<std::vector<char>> empty(BOARDSIZE_Y, std::vector<char>(BOARDSIZE_X));
            board = empty;
        }
};

