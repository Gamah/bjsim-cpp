namespace strategies{
    enum class decisions{HIT, STAND, SPLIT, DOUBLE, SURRENDER};

    class dealerH17{
        decisions play(gameObjects::hand);
    };

    class dealerS17{
        decisions play(gameObjects::hand);
    };

}