#define UH PB1
#define VH PB2
#define WH PB3
#define UL PD2
#define VL PD3
#define WL PD4

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte HallValue, Fill;

void setup()
{
  //Ports setting
  DDRB |= (1 << UH) | (1 << VH) | (1 << WH);
  PORTB = (0 << UH) | (0 << UL) | (0 << WH);
  DDRD |= (1 << UL) | (1 << VL) | (1 << WL);
  PORTD = (0 << UL) | (0 << VL) | (0 << WL);

  //Setting the interrupt from changing the pin state
  PCICR = (1 << PCIE2);
  PCMSK2 = (1 << PCINT23) | (1 << PCINT22) | (1 << PCINT21);

  //ADC setting
  ADMUX |= (1 << REFS0) | (1 << ADLAR); //REFS0 - A Vcc with external capacitor at AREF PIN, ADLAR - left adjust result
  ADCSRA |= (1 << ADEN) | (1 << ADPS2); //ADEN - Turn on ADC, ADPS2 - prescalera to 16, divided clock speed by 16 and we get 1MHz

  //Timers setting
  TCCR1A = 0;           //
  TCCR1B = (1 << CS10); // without prescaler

  TCCR2A = 0;
  TCCR2B = (1 << CS20); //without prescaler

  HallValue = (PIND >> 5) & 7; //zapisanie do zmiennej typu byte stanu pinów 5, 6, 7.
  comutate();

  //lcd
  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("PWM = ");
}

ISR(PCINT2_vect)
{
  HallValue = (PIND >> 5) & 7;
  comutate();
}

//comutate function
void comutate()
{
  switch (HallValue)
  {

  case 1:
    PORTD &= ~0b00001100;
    PORTD |= 0b00010000;
    TCCR2A = 0;          //Turn on pins 10 and 11
    TCCR1A = 0b10000001; //Phase corrected PWM on pin 9
    break;

  case 2:
    PORTD &= ~0b00011000;
    PORTD |= 0b00000100;
    TCCR2A = 0;          //Turn off pins 9 and 11
    TCCR1A = 0b00100001; //Phase corrected PWM on pin 10
    break;

  case 3:
    PORTD &= ~0b00001100;
    PORTD |= 0b00010000;
    TCCR2A = 0;          //Turn on pins 9 and 11
    TCCR1A = 0b00100001; //Phase corrected PWM on pin 10
    break;

  case 4:
    PORTD &= ~0b00010100;
    PORTD |= 0b00001000;
    TCCR1A = 0;          //Turn off pins 9 and 10
    TCCR2A = 0b10000001; //Phase corrected PWM on pin 11
    break;

  case 5:
    PORTD &= ~0b00010100;
    PORTD |= 0b00001000;
    TCCR2A = 0;          //Turn on pins 10 and 11
    TCCR1A = 0b10000001; //Phase corrected PWM on pin 9
    break;

  case 6:
    PORTD &= ~0b00011000;
    PORTD |= 0b00000100;
    TCCR1A = 0;          //Turn off pins 9 and 10
    TCCR2A = 0b10000001; //Phase corrected PWM on pin 11
    break;

  default:
    PORTD = 0;
    break;
  }
}

void loop()
{
  ADCSRA |= (1 << ADSC); //Begin conversion
  while (ADCSRA & (1 << ADSC))
    ;          //Wait until conversion done
  Fill = ADCH; //Get conversion result from ADCH register and assign to Fill variable
  //Assign Fill to pins which control top transistors
  OCR1A = Fill;
  OCR1B = Fill;
  OCR2A = Fill;

  //Display data on LCD screen
  int pwm = Fill;
  pwm = map(pwm, 0, 255, 0, 100);
  lcd.setCursor(6, 0);
  lcd.print("   ");
  lcd.setCursor(6, 0);
  lcd.print(pwm);
  lcd.setCursor(9, 0);
  lcd.print("%");
}
