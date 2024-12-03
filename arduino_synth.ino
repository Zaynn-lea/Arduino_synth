
/*
  Semi-digital polyphonic arduino synthesizer project


  This file requiers the queue header file

  -------------------------------------------------------------------------------------------------

  made by :
    - Gély Léandre :: https://github.com/Zaynn-lea

  dates :
    - started :      28 / 11 / 2024
    - last updated :  2 / 12 / 2024
*/


#include "queue.h"


// For now only 6 voices of polyphonie as it is just a prototype
#define NBR_VOICES 6

// For testing purpuses, I only have a small keyboard
#define NBR_KEYS 6

// Key states 
#define OFF     -1
#define PENDING -2


// Frequences are in milli-Hz to not have to use doubles
const long int Frequencies[] {
   55000,    /* A  1 */    110000,    /* A  2 */    220000,    /* A  3 */    440000,    /* A  4 */
   58250,    /* A# 1 */    116500,    /* A# 2 */    233000,    /* A# 3 */    466000,    /* A# 4 */
   61750,    /* B  1 */    123500,    /* B  2 */    247000,    /* B  3 */    494000,    /* B  4 */
   65375,    /* C  1 */    130750,    /* C  2 */    261500,    /* C  3 */    523000,    /* C  4 */
   69250,    /* C# 1 */    138500,    /* C# 2 */    277000,    /* C# 3 */    554000,    /* C# 4 */
   73375,    /* D  1 */    146750,    /* D  2 */    293500,    /* D  3 */    587000,    /* D  4 */
   77750,    /* D# 1 */    155500,    /* D# 2 */    311000,    /* D# 3 */    622000,    /* D# 4 */
   82375,    /* E  1 */    164750,    /* E  2 */    329500,    /* E  3 */    659000,    /* E  4 */
   87250,    /* F  1 */    174500,    /* F  2 */    349000,    /* F  3 */    698000,    /* F  4 */
   42500,    /* F# 1 */    185000,    /* F# 2 */    370000,    /* F# 3 */    740000,    /* F# 4 */
   98000,    /* G  1 */    196000,    /* G  2 */    392000,    /* G  3 */    784000,    /* G  4 */
  102775,    /* G# 1 */    207750,    /* G# 2 */    415500,    /* G# 3 */    831000,    /* G# 4 */
};


/*
  ++--------------++
  ||  Structures  ||
  ++--------------++
*/

struct zVoice
{
  int  iPinout;
  char cFreqIndex;
  char cValue;
};
typedef struct zVoice sVoice;


struct zKeys
{
  int  iPinout;
  char cFreqIndex;
  char cIsPressed;    // Can be OFF (-1) , PENDING (-2) or the index of the voice it's controllling
};
typedef struct zKeys sKeys;



// general variable

Queue<char> pressedKeys;      // Star of the polyphony *w*

Queue<char> availibleVoices;


unsigned long ulLastTime, ulLastTimeUnupdated;

char cUsedVoices;
char i;


sVoice aVoices[NBR_VOICES];

sKeys aKeyboard[NBR_KEYS];


/*
  ++---------------++
  ||   Functions   ||
  ++---------------++
*/

long int fFreq2Microsecs(long int freq)
{
  /*
    Turns Frequences into a micro seconds for delays

    frequences are in milli-Hz (1000th of Hz)
  */

  return ( long int ) (1e9 / ( double ) freq);
}


void fUpdateKeysStatus()
{
  /*
    Scan the keyboard and update the ON/OFF status of the keys
  */

  for ( i = 0 ; i < NBR_KEYS ; ++i )
  {
    if (digitalRead(aKeyboard[i].iPinout))
    {
      aKeyboard[i].cIsPressed = PENDING;
      push(i, &pressedKeys);
    }
    else if (aKeyboard[i].cIsPressed == PENDING)
    {
      aKeyboard[i].cIsPressed = OFF;
      removeEltQueue(&pressedKeys, i);
    }
    else if (aKeyboard[i].cIsPressed >= 0)
    {
      // send the off command to the voice
      aKeyboard[i].cIsPressed = OFF;
      
      // since we don't have EG/ADSR yet, we just stop sending output and make the voice availible
      aVoices[aKeyboard[i].cIsPressed].cValue = 0;
      push(i, &availibleVoices);
      --cUsedVoices;
    }
  }
}


void fAssignVoices()
{
  /*
    Assigns availible voices to pressed keys
  */

  while (( ! isEmptyQueue(availibleVoices)) && ( ! isEmptyQueue(pressedKeys)))
  {
    aKeyboard[head(pressedKeys)].cIsPressed = head(availibleVoices);
    
    pop(&pressedKeys);
    pop(&availibleVoices);

    ++cUsedVoices;
  }
}


void fWriteToVoices()
{
  /*
    Write the correct level output to the selected voice
  */
  
  for ( i = 0; i < NBR_VOICES ; ++i )
  {
    analogWrite(aVoices[i].iPinout, aVoices[i].cValue);
  }
}

/*
  ++-----------++
  ||   Setup   ||
  ++-----------++
*/

void setup()
{ 
  ulLastTime          = 0;
  ulLastTimeUnupdated = 0;
  cUsedVoices         = 0;

  pressedKeys     = createEmptyQueue('a');
  availibleVoices = createEmptyQueue('a');

  // Initialising all voices
  for (i = 0 ; i < NBR_VOICES ; ++i)
  {
    aVoices[i].iPinout    = i + 2;
    aVoices[i].cFreqIndex = 27;       // All initioalised to the middle C
    aVoices[i].cValue     = 0;
    
    pinMode(aVoices[i].iPinout, OUTPUT);
    
    // At the start of the machine, all voices are availible
    push(i, &availibleVoices);
  }

  // Initialising the keyboard
  for (i = 0 ; i < NBR_KEYS ; ++i)
  {
    aKeyboard[i].iPinout    = i + 22;
    aKeyboard[i].cFreqIndex = i + 12;   // Offset by one octave
    aKeyboard[i].cIsPressed = OFF;
    
    pinMode(aKeyboard[i].iPinout, INPUT);
  }
}

/*
  ++---------------++
  ||   Main code   ||
  ++---------------++
*/

void loop()
{
  // Time meusure in Microsecond to get thoses hight notes going
  ulLastTime  = micros();

  // Scaning the keyboard
  fUpdateKeysStatus();

  // Assigning voices
  if ( ! isEmptyQueue(availibleVoices))
  {
    fAssignVoices();
  }
  
  // Writing updates to all voices
  fWriteToVoices();

  ulLastTimeUnupdated = ulLastTime;
}
