#include <vector>
#include <random>

class shoe{
    public:
        std::vector<int> cards;
        int runningCount;
        int downCard;

        void shuffleCards(std::mt19937& rengine);
        void updateRunningCount(int cardValue);
        int getCard();
        int getDownCard();
        void flipDownCard();
        int trueCount();
};