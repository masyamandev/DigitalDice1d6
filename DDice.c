#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define F_CPU 4800000UL  // 4 MHz

#define DDR_MASK 0b00000000
#define PORT_MASK 0b00000001

#define DICE_POSITIONS 6
const uint8_t POSITION_LEDS[DICE_POSITIONS] =
	{0b00001000, 0b01000001, 0b00101010, 0b01100011, 0b01101011, 0b01110111};

#define LEDS 7
#define PIN_DDR 0
#define PIN_OUT 1
const uint8_t PIN_LEDS[LEDS][2] = {
		{0b00010100, 0b00000100},
		{0b00001010, 0b00001000},
		{0b00011000, 0b00010000},
		{0b00001010, 0b00000010},
		{0b00000110, 0b00000100},
		{0b00011000, 0b00001000},
		{0b00000110, 0b00000010}
	};



uint8_t ledsShowed;
uint8_t currentLed;

volatile uint8_t randomCounter;

#define ANIMATION_FRAMES 8
const uint8_t ANIMATION_LEDS[ANIMATION_FRAMES + 1] =
	{0b00001000, 0b00000010, 0b00010000, 0b01000000, 0b00001000, 0b0000001, 0b00000100, 0b00100000, 0b00001000};

#define ANIMATION_DELAY (F_CPU / 256UL / 20UL)
uint16_t animationDelay;
uint8_t animationFrame;
uint8_t animationEnabled;

#define SLEEP_DELAY (30UL * F_CPU / 256UL)
volatile uint32_t sleepCounter;

inline void clearLEDS();

int main(void)
{
	cli();
	
	//clearLEDS();
	randomCounter = 0;
	currentLed = 0;
	ledsShowed = 0b01111111; // show all
	// reset animation
	animationFrame = 0;
	animationDelay = 0;
	animationEnabled = 1;
	// reset sleep counter
	sleepCounter = 0;

	// init timer
	// prescale timer to 1/1st the clock rate
	TCCR0B |= (1<<CS00);
	// enable timer overflow interrupt
	TIMSK0 |= (1<<TOIE0);
	
	// init pc0 interrupt
	// Set the falling edge of INT0 generates an interrupt request.
	//MCUCR |= (1 << ISC01) | (0 << ISC00);
	// External Interrupt Request 0 Enable
	GIMSK |= (1 << PCIE);
	// Set pin mask
	PCMSK = (1 << PCINT0);
	
	sei();
	
	while(1)
	{
		if (sleepCounter >= SLEEP_DELAY) {
			uint8_t ledsBeforeSleep = ledsShowed;
			// go to sleep mode
			clearLEDS();
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();
			sleep_cpu(); 
			sleep_disable();
			// restore previous state
			ledsShowed = ledsBeforeSleep;
			animationEnabled = 0;
		}
	}
}

inline void clearLEDS() 
{
	DDRB = DDR_MASK;
	PORTB = PORT_MASK;
}

inline void showLed(uint8_t config[]) 
{
	DDRB |= config[PIN_DDR];
	PORTB |= config[PIN_OUT];
}

inline uint8_t isButtonPressed()
{
	return (PINB & (1 << PB0)) == 0;
}


inline uint8_t isAnimationInProgress() 
{
	return (animationFrame <= ANIMATION_FRAMES) || isButtonPressed();
}

inline uint8_t getAnimationLeds()
{
	// start new animation
	if (animationFrame > ANIMATION_FRAMES)
	{
		animationFrame = 0;
		animationDelay = 0;
	}
	
	uint8_t currentFrame = ANIMATION_LEDS[animationFrame];
	
	// calculate next frame
	animationDelay ++;
	if (animationDelay > ANIMATION_DELAY)
	{
		animationDelay = 0;
		animationFrame ++;
		if (animationFrame >= ANIMATION_FRAMES && isButtonPressed())
		{
			animationFrame = 0;
		}
	}
	
	return currentFrame;
}

// Timer interrupt
ISR(TIM0_OVF_vect)
{
	if (sleepCounter >= SLEEP_DELAY) {
		return;
	}
	
	randomCounter ++;
	if (randomCounter >= DICE_POSITIONS)
	{
		randomCounter = 0;
	}

	currentLed ++;
	if (currentLed >= LEDS)
		currentLed = 0;
	
	uint8_t ledsToDisplay = isAnimationInProgress() && animationEnabled ? getAnimationLeds() : ledsShowed;
	
	clearLEDS();
	if (ledsToDisplay & (1 << currentLed)) 
	{
		showLed(PIN_LEDS[currentLed]);
	}
	
	sleepCounter ++;
}

// Button pressed interrupt
ISR(PCINT0_vect)
{ 
	if (isButtonPressed())
	{
		ledsShowed = POSITION_LEDS[randomCounter];
		// reset sleep mode
		sleepCounter = 0;
		animationEnabled = 1;
	}
}