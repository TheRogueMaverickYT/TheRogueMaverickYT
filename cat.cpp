#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>

using namespace std::literals;

void typePrint(const std::string &s, int ms = 30) {
    for (char c : s) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    std::cout << std::endl;
}

void pauseMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
    // ANSI colors (works in most terminals)
    const char* RST = "\033[0m";
    const char* YEL = "\033[33m";
    const char* CYN = "\033[36m";
    const char* MAG = "\033[35m";

    std::mt19937 rng((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> d(0, 2);

    std::cout << CYN;
    typePrint("A little story about a cat named Psycho", 40);
    std::cout << RST;
    pauseMs(300);

    std::cout << YEL;
    typePrint(R"(
 /\_/\  
( o.o )  Psycho
 > ^ <
)", 4);
    std::cout << RST;
    pauseMs(300);

    std::vector<std::string> beats = {
        "Psycho wakes on the windowsill, crowned by sunlight.",
        "He studies a dust mote as if it's the last mystery on Earth.",
        "Suddenly: a daring leap onto the bookshelf. A small victory.",
        "When the house is quiet, Psycho patrols the kitchen like a tiny, furry general.",
        "At night he becomes a whisper—soft paws, sharp curiosity."
    };

    for (const auto &line : beats) {
        typePrint(line, 28 + d(rng) * 10);
        pauseMs(350);
    }

    typePrint(MAG ("Then Psycho finds an abandoned sock.") RST, 28);
    pauseMs(400);
    typePrint("He stares at it. He pounces. The sock trembles under his tiny reign.", 28);
    pauseMs(500);

    std::vector<std::string> endings = {
        "He curls up on the sock and dreams of grand adventures.",
        "He dozes, dreaming of galaxies made of yarn.",
        "He blinks slowly, satisfied, the world correctly arranged around him."
    };
    typePrint(endings[d(rng)], 30);
    pauseMs(500);

    typePrint("The end. For now.", 40);
    return 0;
}