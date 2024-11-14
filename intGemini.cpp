#include <iostream>
#include <gemini/gemini.h> // Include the Gemini library header

int main() {
    // Initialize Gemini
    gemini::Gemini geminiInstance;

    // Example usage of Gemini
    geminiInstance.initialize();
    std::cout << "Gemini initialized successfully!" << std::endl;

    // Perform some operations with Gemini
    // ...

    // Clean up and shutdown Gemini
    geminiInstance.shutdown();
    std::cout << "Gemini shutdown successfully!" << std::endl;

    return 0;
}