#include <iostream>
#include <string>
#include <unordered_map>
// Include Gemini library
#include <gemini/gemini.h>
class Chatbot {
public:
    Chatbot() {
        responses["hello"] = "Hi there! How can I help you today?";
        responses["how are you"] = "I'm just a bunch of code, but I'm here to assist you!";
        responses["what is your name"] = "I am GitHub Copilot, your AI assistant.";
        responses["bye"] = "Goodbye! Have a great day!";
    }

    std::string getResponse(const std::string& input) {
        std::string lowerInput = toLowerCase(input);
        if (responses.find(lowerInput) != responses.end()) {
            return responses[lowerInput];
        } else {
            return "I'm sorry, I don't understand that.";
        }
    }

private:
    std::unordered_map<std::string, std::string> responses;

    std::string toLowerCase(const std::string& str) {
        std::string lowerStr = str;
        for (char& c : lowerStr) {
            c = tolower(c);
        }
        return lowerStr;
    }
};

int main() {
    Chatbot chatbot;
    std::string userInput;

    std::cout << "Chatbot: Hello! Type 'bye' to exit the chat." << std::endl;

    while (true) {
        std::cout << "You: ";
        std::getline(std::cin, userInput);

        if (userInput == "bye") {
            std::cout << "Chatbot: " << chatbot.getResponse(userInput) << std::endl;
            break;
        }

        std::cout << "Chatbot: " << chatbot.getResponse(userInput) << std::endl;
    }

    return 0;
}