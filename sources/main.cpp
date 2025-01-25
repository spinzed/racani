#include "examples/ExampleGame.h"
#include "examples/ExampleBSpline.h"
#include "examples/ExampleParticles.h"

#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    std::string execDirectory(argv[0], 0, std::string(argv[0]).find_last_of("\\/"));

    std::string choice;
    if (argc < 2) {
        std::cout << "Please provide the name of the example via command line arguments." << std::endl;
        std::cout << "Available examples: bspline, particles, game" << std::endl;
        return 0;
    } else {
        choice = std::string(argv[1]);
    }

    if (choice == "bspline") {
        ExampleBSpline().run(execDirectory);
    } else if (choice == "particles") {
        ExampleParticles().run(execDirectory);
    } else if (choice == "game") {
        ExampleGame().run(execDirectory);
    } else {
        std::cout << "Invalid example, please provide a valid one." << std::endl;
        std::cout << "Available examples: bspline, particles, game" << std::endl;
    }
}
