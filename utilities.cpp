
namespace gameObjects{

    class card{
        private:
            int values[13] = {1,2,3,4,5,6,7,8,9,10,10,10,10};
            std::string faces[13] = {"A","2","3","4","5","6","7","8","9","T","J","Q","K"};
            std::string suits[4] = {"H","D","S","C"};
            std::string decks[8] = {"1","2","3","4","5","6","7","8"};

        public: 
            int value(int cardIndex){
                return values[cardIndex%13];   
            }

            std::string face(int cardIndex){
                return faces[cardIndex % 13];
            }

            std::string suit(int cardIndex){
                return suits[(cardIndex % 52) / 13];
            }

            std::string deck(int cardIndex){
                return decks[cardIndex / 52];
            }

            std::string print(int cardIndex){
                return "[" + deck(cardIndex) + face(cardIndex) + suit(cardIndex) + "]";
            }
    };

    class hand{
        public:
            std::vector<int> cards;
            gameObjects::card card;
            int total = 0;
            bool isPair = 0;
            bool isSoft = 0;
            bool isSplit = 0;
            bool isDoubled = 0;
            int bet = 0;
            int canSplit = 0;
            bool canDouble = 0;

            void discard(){
              cards.clear();
              total = 0;
              isPair = 0;
              isSoft = 0;  
              isSplit = 0;
              isDoubled = 0;
              bet = 0;
              canSplit = 0;
              canDouble = 0;
            };

            void addCard(int cardIndex){
                //add the card to the list of cards
                int cardValue = card.value(cardIndex);
                cards.push_back(cardIndex);
                //if this is the 2nd card, we need to check for pairs
                if (cards.size() == 2){
                    //check if the card coubled matches the total plus the card for a double
                    //also check if a previous soft 11 (single ace) plus the incoming card (ace) match by adding up to 12
                    if (total + cardValue == cardValue * 2 || (total + cardValue == 12 && cardValue == 1)){
                        isPair = 1;
                        //if they're aces, it's a soft hand
                        if (cardValue == 1){
                            isSoft = 1;
                        }else{
                            isSoft = 0;
                        }

                    }   
                total = total + cardValue;
                }else{
                    isPair = 0;
                    //if the current card is an ace and the hand total is less than 12, lets add 11 instead of 1
                    total = total + cardValue;
                    if(total < 12 && cardValue == 1){
                        total = total + 10;
                        isSoft = 1;
                    }
                    if(total > 21 && isSoft == 1){
                        total = total - 10;
                        isSoft = 0;
                    }
                }
                return;
            }
    };

    class player{
        public:
            std::vector<gameObjects::hand> hands;
            std::string name;
            int bankroll = 0;
            strategies::dealerH17 strategy;
            int betUnit = 0;
            int betMultiplier = 0;
    };


    
}