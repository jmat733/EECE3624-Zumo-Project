
#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define FAST_HOLD_TICKS 0

void UART0_Init(void)
{
	UBRR0H = 0;
	UBRR0L = 47;

	UCSR0A = 0;
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART0_SendChar(char data)
{
	while (!(UCSR0A & (1 << UDRE0)));

	UDR0 = data;
}

void Joystick_Init(void)
{
	// PB0 RIGHT
	// PB1 DOWN
	// PB2 LEFT
	// PB3 UP
	// PB4 CENTER

	DDRB &= ~0x1F;
	PORTB |= 0x1F;
}

char ReadJoystick(void)
{
	uint8_t buttons = PINB;

	// CENTER has priority
	if (!(buttons & (1 << PB4)))
	return 'S';

	if (!(buttons & (1 << PB2)))
	return 'L';

	if (!(buttons & (1 << PB0)))
	return 'R';

	if (!(buttons & (1 << PB3)))
	return 'F';

	if (!(buttons & (1 << PB1)))
	return 'B';

	return 'S';
}

int main(void)
{
	UART0_Init();
	Joystick_Init();

	char lastReading = 'S';
	char stableCommand = 'S';

	uint8_t stableCount = 0;
	uint8_t sendCounter = 0;

	uint16_t centerHoldCount = 0;
	uint8_t centerHandled = 0;

	uint8_t fastMode = 0;

	// Normal mode at startup
	UART0_SendChar('N');
	UART0_SendChar('S');

	while (1)
	{
		uint8_t buttons = PINB;

		// CENTER button held LOW
		if (!(buttons & (1 << PB4)))
		{
			if (centerHoldCount < FAST_HOLD_TICKS)
			centerHoldCount++;

			// Toggle once per long press
			if (centerHoldCount >= FAST_HOLD_TICKS
			&& !centerHandled)
			{
				fastMode = !fastMode;

				if (fastMode)
				UART0_SendChar('T');
				else
				UART0_SendChar('N');

				centerHandled = 1;
			}
		}
		else
		{
			centerHoldCount = 0;
			centerHandled = 0;
		}

		char reading = ReadJoystick();

		// Debounce
		if (reading == lastReading)
		{
			if (stableCount < 3)
			stableCount++;
		}
		else
		{
			stableCount = 0;
			lastReading = reading;
		}

		// Transmit new movement command
		if (stableCount >= 2 &&
		reading != stableCommand)
		{
			stableCommand = reading;

			UART0_SendChar(stableCommand);
			sendCounter = 0;
		}

		// Repeat movement every 80 ms
		sendCounter++;

		if (sendCounter >= 8)
		{
			UART0_SendChar(stableCommand);
			sendCounter = 0;
		}

		_delay_ms(10);
	}
}
