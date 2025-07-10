// Assistant.cpp
#define _CRT_SECURE_NO_WARNINGS
#include "Assistant.h"

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <sstream>

std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

bool Contains(const std::string& input, const std::string& keyword) {
    return input.find(keyword) != std::string::npos;
}

void SaveNote(const std::string& note) {
    std::ofstream file("notes.txt", std::ios::app);
    if (file.is_open()) file << note << "\n";
}

void SaveTask(const std::string& task) {
    std::ofstream file("tasks.txt", std::ios::app);
    if (file.is_open()) file << task << "\n";
}

void ClearMemory() {
    std::ofstream notes("notes.txt", std::ios::trunc);
    std::ofstream tasks("tasks.txt", std::ios::trunc);
}

std::string LoadMemory() {
    std::ifstream notes("notes.txt");
    std::ifstream tasks("tasks.txt");
    std::stringstream out;

    std::string line;
    out << "NOTES:\n";
    while (getline(notes, line)) out << "- " << line << "\n";

    out << "\nTASKS:\n";
    while (getline(tasks, line)) out << "- " << line << "\n";

    return out.str();
}

std::string ProcessCommand(const std::string& input_raw) {
    std::string input = ToLower(input_raw);

    std::map<std::string, std::vector<std::string>> intents = {
        {"joke",     {"joke", "funny", "laugh"}},
        {"time",     {"time", "clock", "now"}},
        {"greeting", {"hello", "hi", "hey", "morning", "evening"}},
        {"note",     {"note", "remember", "write"}},
        {"task",     {"remind", "task", "todo"}},
        {"who",      {"who are you", "name", "what are you"}},
        {"sad",      {"sad", "depressed", "tired", "upset"}},
        {"help",     {"help", "can you do"}},
        {"clear",    {"clear", "reset memory"}},
        {"show",     {"show notes", "show tasks", "my list"}},
        {"exit",     {"exit", "quit", "close"}}
    };

    std::string detectedIntent = "unknown";

    for (const auto& intentPair : intents) {
        const std::string& intent = intentPair.first;
        const std::vector<std::string>& keywords = intentPair.second;

        for (const std::string& word : keywords) {
            if (Contains(input, word)) {
                detectedIntent = intent;
                break;
            }
        }
        if (detectedIntent != "unknown") break;
    }

    if (detectedIntent == "joke") {
        return "Why don't robots take naps? They recharge instead.";
    }
    else if (detectedIntent == "time") {
        time_t now = time(0);
        char dt[26];
        ctime_s(dt, sizeof(dt), &now);
        return "Current time is: " + std::string(dt);
    }
    else if (detectedIntent == "greeting") {
        return "Hello! I’m Nova, your AI assistant. How can I help today?";
    }
    else if (detectedIntent == "note") {
        SaveNote(input_raw);
        return "Your note has been saved.";
    }
    else if (detectedIntent == "task") {
        SaveTask(input_raw);
        return "Task added to your list.";
    }
    else if (detectedIntent == "who") {
        return "I’m Nova, your offline smart assistant built with C++. I help you stay organized.";
    }
    else if (detectedIntent == "sad") {
        return "I’m here for you. Want to take a deep breath together?";
    }
    else if (detectedIntent == "help") {
        return "I can help you with tasks, notes, jokes, time, and more. Just type your command.";
    }
    else if (detectedIntent == "clear") {
        ClearMemory();
        return "Memory cleared. All tasks and notes deleted.";
    }
    else if (detectedIntent == "show") {
        return LoadMemory();
    }
    else if (detectedIntent == "exit") {
        exit(0);
    }
    else {
        return "Sorry, I didn’t understand that. Try asking me something like 'remind me', 'note', 'joke', or 'what's the time'.";
    }
}
