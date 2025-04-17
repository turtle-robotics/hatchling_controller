#ifndef TURTLE_CONTROLLER
#define TURTLE_CONTROLLER

#include <Arduino.h>
#include <ezButton.h>

class Joystick{
    private:
        int xPin, yPin;
        ezButton zButton;

    public:
        Joystick(); // default constructor
        Joystick(int xPin, int yPin, int zPin); // constructor
        ~Joystick(); // destructor
        //Joystick(const Joystick& other); // copy constructor
        //Joystick& operator=(const Joystick& other); // copy assignment operator

        float getX(float deadzone);
        float getY(float deadzone);
        bool getZ();
        void joystickUpdate();
    private:
        void setup();

};


class Controller{
    private:
        Joystick joy1;
        ezButton butA, butB, butX, butY, butR, butL, butSwitch;
    
    public:
        Controller(); // default constructor
        Controller(int j1x, int j1y, int j1z, int butAPin, int butBPin, int butXPin, int butYPin, int butRPin, int butLPin, int butSPin);
        ~Controller();

        float getJoy1X(float deadzone = 0);
        float getJoy1Y(float deadzone = 0);

        bool getJoy1Z();

        bool getA();
        bool getB();
        bool getX();
        bool getY();
        bool getR();
        bool getL();
        bool getS();

        void controllerUpdate();

    private:
        void setup();

        

        

        

};



#endif