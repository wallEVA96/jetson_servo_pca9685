/*
Copyright (c) 2012-2017 Ben Croston ben@croston.org.
Copyright (c) 2019, NVIDIA CORPORATION.
Copyright (c) 2019 Jueon Park(pjueon) bluegbg@gmail.com.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
// for delay function.
#include <chrono>
#include <thread>

// for signal handling
#include <signal.h>

#include <JetsonGPIO.h>

#include <JHPWMPCA9685.h>

using namespace std;

// static bool end_this_program = false;

inline void delay(int s) { this_thread::sleep_for(chrono::seconds(s)); }
inline void mdelay(int ms) { this_thread::sleep_for(chrono::milliseconds(ms)); }
void signalHandler(int s) { /* end_this_program = true; */  exit(0); }

/**
 * @brief ServoPCA9685 Class
 */
class auto_gunsight
{
public:
	int oe_pin;
    int pca9685_addr;	
	int nx_i2c_bus;
	PCA9685 *servo_pca9685;
	
	auto_gunsight();
	~auto_gunsight();
	int init_gunsight();
	void setup_oe_pin();
	void cleanup_oe_pin();
	void enable_pca9685();
	void disable_pca9685();
	void setup_servo_pwm(int ch0, int ch1, int ch2);
};

/**
 * @brief constrcut function.
 */
auto_gunsight::auto_gunsight():oe_pin(7),pca9685_addr(0x40),nx_i2c_bus(8)
{
	servo_pca9685 = new PCA9685(pca9685_addr, nx_i2c_bus);

}

/**
 * @brief dec
 */
auto_gunsight::~auto_gunsight()
{
	disable_pca9685();
	cleanup_oe_pin();
	delete servo_pca9685;
}

/**
 * @brief init_gunsight 
 */
int auto_gunsight::init_gunsight()
{
	setup_oe_pin();

	if(servo_pca9685->openPCA9685())
		cout << "INFO: Open I2C Bus " << nx_i2c_bus << " Successfully" << endl;
	else
	{
		cout << "INFO: Failed To Open I2C Bus " << nx_i2c_bus << endl;
		return -1;
	}

	const float pwm_fre = 46.80; //46.80;

	servo_pca9685->setPWMFrequency(pwm_fre); //near 50hz.
	enable_pca9685();
	
	return 0;
}

/**
 * @brief setup_oe_pin
 */
void auto_gunsight::setup_oe_pin()
{
    // Pin Setup.
    GPIO::setmode(GPIO::BOARD);
    // set pin as an output pin with optional initial state of HIGH
    GPIO::setup(oe_pin, GPIO::OUT, GPIO::HIGH);
}

/**
 * @brief cleanup_oe_pin
 */
void auto_gunsight::cleanup_oe_pin()
{
    GPIO::cleanup();
}

/**
 * @brief enable_pca9685
 */
void auto_gunsight::enable_pca9685()
{
    GPIO::output(oe_pin, GPIO::LOW);
}

/**
 * @brief disable_pca9685 
 */
void auto_gunsight::disable_pca9685()
{
    GPIO::output(oe_pin, GPIO::HIGH);
}

/**
 * @brief setup_servo_pwm
 *
 * @param ch0
 * @param ch1
 * @param ch2
 */
void auto_gunsight::setup_servo_pwm(int ch0, int ch1, int ch2)
{
	if(ch0 >= 4096)  ch0 = 4095;
	if(ch1 >= 4096)  ch1 = 4095;
	if(ch2 >= 4096)  ch2 = 4095;

	if(ch0 < 0)  ch0 = 0;
	if(ch1 < 0)  ch1 = 0;
	if(ch2 < 0)  ch2 = 0;

	servo_pca9685->setPWM(0, 0, ch0); //5us 50hz.
	servo_pca9685->setPWM(1, 0, ch1);
	servo_pca9685->setPWM(2, 0, ch2);

	// 20 / 4096 = 0.00488, 102 ~ 521, 180: 0.42',  360: 0.84'
	int rev = 0;
	int inc = 101;
	while(1)
	{
		if(inc < 102) rev = 1;
		if(inc > 521) rev = -1;
		inc += rev;

		servo_pca9685->setPWM(0, 0, inc); //5us
		mdelay(10);
	}
}

/**
 * @brief main entry.
 *
 * @return err 
 */
int main()
{
    // When CTRL+C pressed, signalHandler will be called
    signal(SIGINT, signalHandler);
	
	auto_gunsight bulls_eye;
	if(bulls_eye.init_gunsight() == -1)
		return -1;

	bulls_eye.setup_servo_pwm(-10, 5000, 1);

//    while (!end_this_program)
//    {
//        delay(1);
//    }

    return 0;
}
