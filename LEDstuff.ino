#define PIN 6  // to leds
#define LIN_OUT 1
#define FHT_N 256
#include <FHT.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "RunningAverage.h"
#define BUFFERSIZE 128

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

byte SampleL[60];

unsigned long interval = 333000;    // max time between beats
unsigned long previousMicros = 0;
unsigned long currentMicros = 0;

int displaySize = 60;
float MaxBrightness = .5;
int MaxLoudness = 1;
float Threshold = .05;
int FadeStep = 10;
int NoiseFloor;
int LowBand = 1;
int MidBand = 4;

uint8_t LowSamples[BUFFERSIZE];
byte getPos = 0;
byte putPos = 0;
byte NextPos = 0;
bool OKFull = 0;

int samples = 0;

uint8_t LowBeatCurrent = 0;
uint8_t LowBeatTimeDelta = 0;
uint8_t LowBeatPrevious = 0;
uint8_t LowSamplePos = 0;

    
void setup() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc0
    DIDR0 = 0x01; // turn off the digital input for adc0

    colorWipe(strip.Color((MaxBrightness * 255), 0, 0), 1);
    colorWipe(strip.Color(0, (MaxBrightness * 255), 0), 1);
    colorWipe(strip.Color(0, 0, (MaxBrightness * 255)), 1);
}

void loop() {
    sampleInput();
    sampleFix();
    
    doAnalyze();    // analyze data, then drawSpectrum
    
    NextPos = (putPos + 1) & (BUFFERSIZE-1);        // our spot in our buffer array
    if (NextPos > BUFFERSIZE-1) NextPos = BUFFERSIZE-1;
    if (NextPos < 0) NextPos = 0;
    putPos = NextPos;
}

void doFade() {
    uint8_t zBrightness = strip.getBrightness();
    if (zBrightness - FadeStep > 2) {
        zBrightness = zBrightness - FadeStep;
    }
    strip.setBrightness(zBrightness);
    strip.show();
}

void drawSpectrum () {
    #ifdef DEBUG
    Serial.println("drawSpectrum()");
    #endif
    
    strip.setBrightness(255);
    for (int i=0; i < displaySize; i++) {
        strip.setPixelColor(i, strip.Color(50, 0, 0));
//        strip.setPixelColor(i, strip.Color(SampleL[i]*MaxBrightness, 0, 0));
    }
    strip.show();
}

void doAnalyze() {
    float zVal;
    uint8_t zLowMax = 0;
    uint16_t zLowTotal = 0;
    float zLowAvg = 0;

    uint8_t zPrevLow = 0;
    uint8_t zPrevPrevLow = 0;
    
    if (OKFull != 1) {              // so the average is based on sampled data, i.e., before the buffer is filled up
        for (int i=0; i < putPos+1; i++) {
            zLowTotal += LowSamples[i];
            if (LowSamples[i] > zLowMax) {
                zLowMax = LowSamples[i];
                LowSamplePos = i;
            }
        }
        zLowAvg = zLowTotal/putPos;
    }
    else {
        for (int i=0; i < BUFFERSIZE; i++) {
            zLowTotal += LowSamples[i];
            if (LowSamples[i] > zLowMax) {
                zLowMax = LowSamples[i];
                LowSamplePos = i;
            }
        }
        zLowAvg = zLowTotal/BUFFERSIZE;
    }
    
    // currentMicros = micros();        // some attempt to check the timing of the beat. unfinished
    // if (SampleL[LowBand] > zLowMax*.9) {
        // LowBeatCurrent = currentMicros;
        // LowBeatTimeDelta = LowBeatCurrent - LowBeatPrevious;
        // LowBeatPrevious = LowBeatCurrent;
    // }
//    if (LowBeatTimeDelta > interval)

    if (putPos == 0) {
        zPrevLow = BUFFERSIZE-1;
        zPrevPrevLow = BUFFERSIZE-2;
    }
    else if ((putPos) == 1) {
        zPrevPrevLow = BUFFERSIZE-1;
    }
    else {
        zPrevLow = putPos-2;
        zPrevPrevLow = putPos-3;
    }
    zPrevLow = LowSamples[zPrevLow];
    zPrevPrevLow = LowSamples[zPrevPrevLow];
    uint8_t zLowMin = 0;
    if (zPrevPrevLow < zPrevLow) {            // get the lower of the two
        zLowMin = zPrevPrevLow;
    }
    else {
        zLowMin = zPrevLow;
    }

    uint8_t zLowOnset = zLowMax - zLowMin;
    if (zLowOnset/zLowMax > .9 && LowSamples[putPos] > zLowAvg) {
        drawSpectrum();
    }
    else {
        doFade();
    }
}

void sampleInput() {    
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int x=0; x<FHT_N; x++) {
        while(!(ADCSRA & 0x10)); // wait for adc to be ready
        ADCSRA = 0xf5; // restart adc
        byte m = ADCL; // fetch adc data
        byte j = ADCH;
        int k = (j << 8) | m; // form into an int
        k -= 0x0200; // form into a signed int
        k <<= 6; // form into a 16b signed int
        fht_input[x] = k; // put real data into bins
    }
//    fht_window(); // window the data for better frequency response (which apparently fucks up the low end, hence being commented out)
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
//    fht_mag_log();
    fht_mag_lin();
    sei();
}

void sampleFix() {   
    int newPos; 
    float fhtCount, tempY;
    int lowBin = 0;
    int midBin = 0;
    for (int x=0; x < displaySize; x++) {
        fhtCount = FHT_N/2;
        newPos = x * (fhtCount / displaySize); 
        tempY = fht_lin_out[newPos]; 
        if ( tempY > NoiseFloor + 20 ) {
            SampleL[x] = ((tempY/(FHT_N/2))*100); }  // 0 to 100% of max brightness, which is a % too
        else {
            SampleL[x] = 0;}
    }

    if (OKFull == 0 && putPos == BUFFERSIZE-1) {
        OKFull = 1;
    }
    
    unsigned long ztemp = micros();
    LowSamples[putPos] = fht_lin_out[LowBand]/(FHT_N/2);         // somewhere around 100 hz
//    MidSamples[putPos] = SampleL[MidBand];         // somewhere around 1000 hz
}  

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}