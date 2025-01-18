#include <iostream>  
#include <vector>  
#include <algorithm>  
#include <random>  
#include <chrono>  
#include <iomanip>  

#ifdef _WIN32  
#include <windows.h>  
void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
void setupConsoleUTF8() {
    SetConsoleOutputCP(CP_UTF8);
}
#else  
#include <unistd.h>  
#include <term.h>  
void SetColor(int color) {
    if (!cur_term) {
        int result;
        setupterm(nullptr, STDOUT_FILENO, &result);
        if (result <= 0) return;
    }
    putp(tigetstr("sgr0"));
    printf("\033[%dm", color);
}
void setupConsoleUTF8() {}
#endif  

using namespace std;

struct Card {
    string suit;
    string rank;
    int value;
};

class Blackjack {
private:
    vector<Card> deck;
    vector<Card> playerHand;
    vector<Card> dealerHand;
    int playerChips;
    int bet;

    void initializeDeck() {
        vector<string> suits = { "C", "D", "T", "P" }; // Cors, Diamants, Trèvols, Piques  
        vector<string> ranks = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
        for (const auto& suit : suits) {
            for (int i = 0; i < ranks.size(); ++i) {
                int value = i + 1;
                if (value > 10) value = 10;
                deck.push_back({ suit, ranks[i], value });
            }
        }
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(deck.begin(), deck.end(), default_random_engine(seed));
    }

    Card dealCard() {
        Card card = deck.back();
        deck.pop_back();
        return card;
    }

    int calculateHandValue(const vector<Card>& hand) {
        int value = 0;
        int aces = 0;
        for (const auto& card : hand) {
            if (card.rank == "A") {
                aces++;
                value += 11;
            }
            else {
                value += card.value;
            }
        }
        while (value > 21 && aces > 0) {
            value -= 10;
            aces--;
        }
        return value;
    }

    void displayCardsSideBySide(const vector<Card>& hand, bool hideFirst = false) {
        vector<string> topBottom, valueTop, middle, suit, valueBottom, lowBottom;
        for (size_t i = 0; i < hand.size(); ++i) {
            if (i == 0 && hideFirst) {
                topBottom.push_back("┌───────┐");
                valueTop.push_back("│   ??  │");
                middle.push_back("│       │");
                suit.push_back("│   ??  │");
                valueBottom.push_back("│   ??  │");
                lowBottom.push_back("└───────┘");
            }
            else {
                topBottom.push_back("┌───────┐");
                valueTop.push_back("│ " + hand[i].rank + string(3 - hand[i].rank.length(), ' ') + "   │");
                middle.push_back("│       │");
                suit.push_back("│   " + hand[i].suit + "   │");
                valueBottom.push_back("│   " + string(3 - hand[i].rank.length(), ' ') + hand[i].rank + " │");
                lowBottom.push_back("└───────┘");
            }
        }
        for (const auto& line : { topBottom, valueTop, middle, suit, middle, valueBottom, lowBottom }) {
            for (const auto& segment : line) {
                cout << segment << " ";
            }
            cout << endl;
        }
    }

public:
    Blackjack() : playerChips(100) {
        initializeDeck();
    }

    void playRound() {
        SetColor(14); // Amarillo  
        cout << "Tienes " << playerChips << " fichas." << endl;
        SetColor(15); // Blanco  

        do {
            cout << "Cuanto quieres apostar? ";
            cin >> bet;
        } while (bet <= 0 || bet > playerChips);

        playerHand.clear();
        dealerHand.clear();
        playerHand.push_back(dealCard());
        dealerHand.push_back(dealCard());
        playerHand.push_back(dealCard());
        dealerHand.push_back(dealCard());

        while (true) {
            SetColor(11); // Cyan  
            cout << "Tu mano: " << endl;
            SetColor(15); // Blanco  
            displayCardsSideBySide(playerHand);
            cout << "Valor de tu mano: " << calculateHandValue(playerHand) << endl;

            SetColor(11); // Cyan  
            cout << "Mano del crupier: " << endl;
            SetColor(15); // Blanco  
            displayCardsSideBySide(dealerHand, true);

            char choice;
            cout << "Quieres otra carta? (s/n): ";
            cin >> choice;
            if (choice == 'n' || choice == 'N') break;

            playerHand.push_back(dealCard());
            if (calculateHandValue(playerHand) > 21) {
                SetColor(12); // Rojo  
                cout << "Tu mano: " << endl;
                displayCardsSideBySide(playerHand);
                cout << "Te has pasado! Pierdes " << bet << " fichas." << endl;
                playerChips -= bet;
                SetColor(15); // Blanco  
                return;
            }
        }

        SetColor(11); // Cyan  
        cout << "Mano del crupier: " << endl;
        SetColor(15); // Blanco  
        displayCardsSideBySide(dealerHand);
        while (calculateHandValue(dealerHand) < 17) {
            dealerHand.push_back(dealCard());
            cout << "El crupier toma una carta: " << endl;
            displayCardsSideBySide(dealerHand);
        }

        cout << "Valor de la mano del crupier: " << calculateHandValue(dealerHand) << endl;
        int playerValue = calculateHandValue(playerHand);
        int dealerValue = calculateHandValue(dealerHand);
        if (dealerValue > 21 || playerValue > dealerValue) {
            SetColor(10); // Verde  
            cout << "Ganaste! Ganas " << bet << " fichas." << endl;
            playerChips += bet;
        }
        else if (playerValue < dealerValue) {
            SetColor(12); // Rojo  
            cout << "Perdiste. Pierdes " << bet << " fichas." << endl;
            playerChips -= bet;
        }
        else {
            SetColor(13); // Magenta  
            cout << "Empate. Recuperas tu apuesta." << endl;
        }
        SetColor(15); // Blanco  
    }

    bool canContinue() {
        return playerChips > 0 && deck.size() >= 10;
    }

    int getPlayerChips() const {
        return playerChips;
    }
};

int main() {
    setupConsoleUTF8(); // Configura la consola para UTF-8 si está disponible  
    SetColor(10); // Verde  
    cout << R"(
        ___           ___           ___                       ___           ___     
       /\  \         /\  \         /\  \          ___        /\__\         /\  \    
      /::\  \       /::\  \       /::\  \        /\  \      /::|  |       /::\  \   
     /:/\:\  \     /:/\:\  \     /:/\ \  \       \:\  \    /:|:|  |      /:/\:\  \  
    /:/  \:\  \   /::\~\:\  \   _\:\~\ \  \      /::\__\  /:/|:|  |__   /:/  \:\  \ 
   /:/__/ \:\__\ /:/\:\ \:\__\ /\ \:\ \ \__\  __/:/\/__/ /:/ |:| /\__\ /:/__/ \:\__\
   \:\  \  \/__/ \/__\:\/:/  / \:\ \:\ \/__/ /\/:/  /    \/__|:|/:/  / \:\  \ /:/  /
    \:\  \            \::/  /   \:\ \:\__\   \::/__/         |:/:/  /   \:\  /:/  / 
     \:\  \           /:/  /     \:\/:/  /    \:\__\         |::/  /     \:\/:/  /  
      \:\__\         /:/  /       \::/  /      \/__/         /:/  /       \::/  /   
       \/__/         \/__/         \/__/                     \/__/         \/__/         
)" << '\n';
    SetColor(10); // Verde  
    cout << "Bienvenido al Blackjack!" << endl;
    SetColor(15); // Blanco  
    Blackjack game;
    char playAgain;
    do {
        game.playRound();
        if (!game.canContinue()) {
            SetColor(12); // Rojo  
            cout << "No puedes seguir jugando. ";
            if (game.getPlayerChips() <= 0) {
                cout << "Te has quedado sin fichas." << endl;
            }
            else {
                cout << "No hay suficientes cartas en el mazo." << endl;
            }
            break;
        }
        cout << "Quieres jugar otra ronda? (s/n): ";
        cin >> playAgain;
    } while (playAgain == 's' || playAgain == 'S');
    cout << "Gracias por jugar. Te vas con " << game.getPlayerChips() << " fichas." << endl;
    SetColor(15); // Blanco  
    return 0;
}
