// GOL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>    // std::for_each
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

#include <Windows.h> // Must be imported after SFML (otherwise it causes problems with "Rect")

#include "FastNoise.h"

#include "rgbhsv.h"
#include "Board.h"

// Game of Life CPP
// Author: Nathan Laha
// License: MIT
// 
// Requires Intel TBB
// Should run on any platform but won't
// work with enscripten for webassembly
//

sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight) {

    // Compares the aspect ratio of the window to the aspect ratio of the view,
    // and sets the view's viewport accordingly in order to archieve a letterbox effect.
    // A new view (with a new viewport set) is returned.

    float windowRatio = windowWidth / (float)windowHeight;
    float viewRatio = view.getSize().x / (float)view.getSize().y;
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
        horizontalSpacing = false;

    // If horizontalSpacing is true, the black bars will appear on the left and right side.
    // Otherwise, the black bars will appear on the top and bottom.

    if (horizontalSpacing) {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }

    else {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));

    return view;
}

/* MAIN */
int main()
{
    uint16_t BOARDSIZE_X = 1000; // Board width
    uint16_t BOARDSIZE_Y = 1000; // Board height
    uint16_t PIXELSIZE = 1; // the size of each individual cell

    sf::Color NEW_COLOR = sf::Color::Blue;
    sf::Color OLD_COLOR = sf::Color::Red;
    sf::Color DEAD_CELL_COLOR = sf::Color::Black;

    uint8_t BRUSH_SIZE = 2; // starting size of interactive brush

    uint8_t FREQUENCY_MULT = 10; // frequency multiplier for noise

    bool USE_FILE = false;

    // Read config file
    std::ifstream cFile("config.txt");
    if (cFile.is_open())
    {
        std::string line;
        while (getline(cFile, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), isspace),
                line.end());
            if (line[0] == '#' || line.empty())
                continue;
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
            if (name == "boardsize") {
                BOARDSIZE_X = std::stoi(value);
                BOARDSIZE_Y = std::stoi(value);
            }
            else if (name == "pixelsize") {
                PIXELSIZE = std::stoi(value);
            }
            else if (name == "noise_frequency") {
                FREQUENCY_MULT = std::stoi(value);
            }
            else if (name == "use_file") {
                if (value == "true") {
                    USE_FILE = true;
                } else {
                    USE_FILE = false; 
                }
            }
        }

    }
    else {
        std::cerr << "Couldn't open config file for reading.\n";
    }

    Board mainBoard(BOARDSIZE_X, BOARDSIZE_Y);

    FastNoise noise; // Create a FastNoise object
    noise.SetNoiseType(FastNoise::SimplexFractal); // Set the desired noise type

    if (USE_FILE) {
        // Load the txt file
        mainBoard.loadFromFile("Board.txt");
    }
    else {
        // Generate a board with FastNoise
        int noffset = 1 + (rand() % 10000); // random noise offset
        for (int x = 0; x < BOARDSIZE_X; x++)
        {
            for (int y = 0; y < BOARDSIZE_Y; y++)
            {
                if (noise.GetNoise(x * FREQUENCY_MULT + noffset, y * FREQUENCY_MULT + noffset) > 0) {
                    mainBoard.setCell(x, y, '#');
                }
                else {
                    mainBoard.setCell(x, y, '.');
                }
            }
        }
    }

    // Init SFML
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    sf::RenderWindow window(sf::VideoMode(PIXELSIZE * BOARDSIZE_X, PIXELSIZE * BOARDSIZE_Y + 50), "Game of Life", sf::Style::Default, settings);

    // Make an SFML view
    sf::View view;
    view.setSize(PIXELSIZE * BOARDSIZE_X, PIXELSIZE * BOARDSIZE_Y + 50);
    view.setCenter(view.getSize().x / 2, view.getSize().y / 2);
    view = getLetterboxView(view, PIXELSIZE * BOARDSIZE_X, PIXELSIZE * BOARDSIZE_Y);

    // Load font for below UI elements
    sf::Font font;
    if (!font.loadFromFile("font.ttf"))
    {
        // error...
        std::cerr << "Error loading font \"font.ttf\" please make sure it's next to the executable";
        window.close();
    }

    // UI Buttons
    sf::Text clearText; // clears the board
    clearText.setStyle(sf::Text::Bold);
    clearText.setFont(font);
    clearText.setString("Clear");
    clearText.setFillColor(sf::Color::White);
    clearText.setCharacterSize(20);
    clearText.setPosition(50.0f, (BOARDSIZE_Y * PIXELSIZE) + 2.0f);

    sf::Text fillText; // fills with noise
    fillText.setStyle(sf::Text::Bold);
    fillText.setFont(font);
    fillText.setString("Fill");
    fillText.setFillColor(sf::Color::White);
    fillText.setCharacterSize(20);
    fillText.setPosition(130.0f, (BOARDSIZE_Y * PIXELSIZE) + 2.0f);

    sf::Text randColorText; // sets the colors to a new random one
    randColorText.setStyle(sf::Text::Bold);
    randColorText.setFont(font);
    randColorText.setString("RColor");
    randColorText.setFillColor(sf::Color::White);
    randColorText.setCharacterSize(20);

    randColorText.setPosition(190.0f, (BOARDSIZE_Y * PIXELSIZE) + 2.0f);

    // Brush size text
    sf::Text bsizeText; // shows brush size
    bsizeText.setStyle(sf::Text::Bold);
    bsizeText.setFont(font);
    bsizeText.setString("BSize: " + std::to_string(BRUSH_SIZE));
    bsizeText.setFillColor(sf::Color::White);
    bsizeText.setCharacterSize(15);
    bsizeText.setPosition(300.0f, (BOARDSIZE_Y * PIXELSIZE) + 2.0f);

    // Generations text
    sf::Text genText; // shows generations
    genText.setStyle(sf::Text::Bold);
    genText.setFont(font);
    genText.setString("Generations: " + std::to_string(mainBoard.generation));
    genText.setFillColor(sf::Color::White);
    genText.setCharacterSize(15);
    genText.setPosition(300.0f, (BOARDSIZE_Y * PIXELSIZE) + 15.0f);

    // Sim delay text
    sf::Text simdText; // shows simulation delay
    simdText.setStyle(sf::Text::Bold);
    simdText.setFont(font);
    simdText.setString("Simulation Delay: " + std::to_string(0));
    simdText.setFillColor(sf::Color::White);
    simdText.setCharacterSize(15);
    simdText.setPosition(300.0f, (BOARDSIZE_Y * PIXELSIZE) + 28.0f);

    // New color preview
    sf::RectangleShape newcolPreview;
    newcolPreview.setSize(sf::Vector2f(40, 20));
    newcolPreview.setPosition(0, (BOARDSIZE_Y * PIXELSIZE));
    newcolPreview.setFillColor(NEW_COLOR);

    // Old color preview
    sf::RectangleShape oldcolPreview;
    oldcolPreview.setSize(sf::Vector2f(40, 20));
    oldcolPreview.setPosition(0, (BOARDSIZE_Y * PIXELSIZE) + 20.0f);
    oldcolPreview.setFillColor(OLD_COLOR);
    
    // Local vars before the main loop
    int bsize = BRUSH_SIZE;
    sf::Color oldC = OLD_COLOR;
    sf::Color newC = NEW_COLOR;
    int simulationDelay = 0;

    // stop/start sim
    bool simRunning = false;

    // Responsive scaling
    int windowWidth = BOARDSIZE_X * PIXELSIZE;
    int windowHeight = BOARDSIZE_Y * PIXELSIZE;

    // make sure we don't press the button multiple times by holding down left click
    bool btnup = true; 
    // make sure we don't stop and start the simulation with a single keypress
    bool keyup = true;

    // Buffer
    sf::Texture buffer;
    buffer.setSmooth(false);
    if (!buffer.create(BOARDSIZE_X * PIXELSIZE, BOARDSIZE_Y * PIXELSIZE))
    {
        // error...
        std::cerr << "Error creating buffer texture";
        window.close();
    }

    sf::Image image;
    image.create(BOARDSIZE_X * PIXELSIZE, BOARDSIZE_Y * PIXELSIZE, sf::Color::Black);

    // Window Update
    while (window.isOpen())
    {

        // Window event update
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window if closed
            if (event.type == sf::Event::Closed)
                window.close();
            // Check if mouse wheel has moved
            else if (event.type == sf::Event::MouseWheelMoved)
            {
                // Check if shift key is pressed
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                {
                    // Make sure we don't have negative sim delay
                    if (simulationDelay >= 0 && (simulationDelay + event.mouseWheel.delta) >= 0) {

                        // set sim delay
                        simulationDelay += event.mouseWheel.delta;
                    }
                    simdText.setString("Simulation Delay: " + std::to_string(simulationDelay));
                }
                else {
                    // Change brush size and make sure we don't have negative brush size
                    if (bsize > 0 && (bsize + event.mouseWheel.delta) >= 1) {
                        bsize += event.mouseWheel.delta;
                    }
                    bsizeText.setString("BSize: " + std::to_string(bsize));
                }
            }
            // Add letterbox bars on resize
            if (event.type == sf::Event::Resized) {
                view = getLetterboxView(view, event.size.width, event.size.height);
                windowWidth = event.size.width;
                windowHeight = event.size.height;
            }

            // Play and pause simulation
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                if (keyup == true) {
                    keyup = false;
                    simRunning = !simRunning;
                }
            }
            else {
                keyup = true;
            }
        }

        window.clear();

        // get mouse position
        sf::Vector2i position = sf::Mouse::getPosition(window);

        double ccol = floor(position.y + ((windowHeight - (BOARDSIZE_Y * PIXELSIZE)) / 2)) / PIXELSIZE;
        double crow = floor(position.x - ((windowWidth - (BOARDSIZE_X * PIXELSIZE)) / 2)) / PIXELSIZE;

        genText.setString("Generations: " + std::to_string(mainBoard.generation));

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            // left click
            // transform the mouse position from window coordinates to world coordinates
            sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            // retrieve the bounding box of the fill text
            sf::FloatRect fillbounds = fillText.getGlobalBounds();

            // retrieve the bounding box of the clear text
            sf::FloatRect clearbounds = clearText.getGlobalBounds();

            // random color
            sf::FloatRect randcbounds = randColorText.getGlobalBounds();

            if (fillbounds.contains(mouse) || clearbounds.contains(mouse) || randcbounds.contains(mouse)) {

                if (btnup == true) {

                    btnup = false;

                    // fill with noise
                    if (fillbounds.contains(mouse))
                    {
                        int noffset = 1 + (rand() % 10000); // random noise offset
                        for (int x = 0; x < BOARDSIZE_X; x++)
                        {
                            for (int y = 0; y < BOARDSIZE_Y; y++)
                            {
                                if (noise.GetNoise(x * FREQUENCY_MULT + noffset, y * FREQUENCY_MULT + noffset) > 0) {
                                    mainBoard.setCell(x, y, '#');
                                }
                                else {
                                    mainBoard.setCell(x, y, '.');
                                }
                            }
                        }
                    }

                    // clear the board
                    if (clearbounds.contains(mouse))
                    {
                        mainBoard.clearBoard();
                    }

                    // random color
                    if (randcbounds.contains(mouse))
                    {
                        // set a random color
                        sf::Color oldrandColor(1 + (rand() % 255), 1 + (rand() % 255), 1 + (rand() % 255));
                        sf::Color newrandColor(1 + (rand() % 255), 1 + (rand() % 255), 1 + (rand() % 255));
                        
                        oldC = oldrandColor;
                        newC = newrandColor;

                        oldcolPreview.setFillColor(oldC);
                        newcolPreview.setFillColor(newC);
                        
                    }
                }
            }
            else {
                // Paint with our brush and specified brush size
                for (int i = -1 * bsize; i <= 1 * bsize; i++)
                {
                    for (int j = -1 * bsize; j <= 1 * bsize; j++)
                    {
                        mainBoard.setCell(crow + i, ccol + j, '#');
                    }
                }
            }
        }
        else {
            btnup = true;
        }

        // Get Start Time
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

        // Create the board with quads and run the loop in parallel across all threads
        tbb::parallel_for(tbb::blocked_range<uint16_t>(0, BOARDSIZE_Y), [&](tbb::blocked_range<uint16_t> ib)
        {
            tbb::parallel_for(tbb::blocked_range<uint16_t>(0, BOARDSIZE_X), [&](tbb::blocked_range<uint16_t> jb)
            {
                // These loops are divided up across all threads
                for (int i = ib.begin(); i < ib.end(); ++i)
                {
                    for (int j = jb.begin(); j < jb.end(); ++j)
                    {
                        // only draw if cell is alive
                        if (mainBoard.getCell(i, j) == '#') {

                            // Make the cool lifetime color vis thing
                            double t = (double)mainBoard.getCellAge(i, j) / (double)5;

                            if (t > 1) {
                                t = 1;
                            }
                            
                            sf::Color output(0, 0, 0);

                            output.r = RgbToHsv(newC).r * (1 - t) + RgbToHsv(oldC).r * t;
                            output.g = RgbToHsv(newC).g * (1 - t) + RgbToHsv(oldC).g * t;
                            output.b = RgbToHsv(newC).b * (1 - t) + RgbToHsv(oldC).b * t;

                            output = HsvToRgb(output);

                            image.setPixel(i, j, output);

                        }
                        else {

                            // Color the dead cells
                            image.setPixel(i, j, DEAD_CELL_COLOR);

                        }
                    }
                }
            });
        });

        // Get End Time
        auto end = std::chrono::system_clock::now();

        auto diff = std::chrono::duration_cast <std::chrono::milliseconds> (end - start).count();
        std::cout << "Time to compute color = " << diff << " Milliseconds" << '\n';
        
        // Get Start Time
        std::chrono::system_clock::time_point start2 = std::chrono::system_clock::now();

        buffer.update(image);

        sf::Sprite bufsprite;
        bufsprite.setTexture(buffer);
        bufsprite.setScale(PIXELSIZE, PIXELSIZE);

        // Letterbox it for widescreen
        window.setView(view);


        // draw
        window.draw(bufsprite);

        // draw ui
        window.draw(clearText);
        window.draw(fillText);
        window.draw(randColorText);
        window.draw(bsizeText);
        window.draw(genText);
        window.draw(simdText);

        window.draw(oldcolPreview);
        window.draw(newcolPreview);

        // Get End Time
        auto end2 = std::chrono::system_clock::now();

        auto diff2 = std::chrono::duration_cast <std::chrono::milliseconds> (end2 - start2).count();
        std::cout << "Time to draw = " << diff2 << " Milliseconds" << '\n';

        // window render
        window.display();

        // Get Start Time
        std::chrono::system_clock::time_point start3 = std::chrono::system_clock::now();

        // advance the simulation
        if (simRunning == true) {
            mainBoard.nextGeneration();
        }

        // Get End Time
        auto end3 = std::chrono::system_clock::now();

        auto diff3 = std::chrono::duration_cast <std::chrono::milliseconds> (end3 - start3).count();
        std::cout << "Time to compute next generation = " << diff3 << " Milliseconds" << '\n';

        // Delay the next simulation based on our delay time
        std::this_thread::sleep_for(std::chrono::milliseconds(simulationDelay));
    }
}