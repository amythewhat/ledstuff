//#define DEBUG

#define PIN 6  // to leds
#define LIN_OUT 1
//#define LOG_OUT 1
#define FHT_N 256
#include <FHT.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "RunningAverage.h"
#define BUFFERSIZE 128

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);


byte SampleL[60];

unsigned long interval = 75000;    // min time to wait before doing stuff
unsigned long previousMicros = 0;
unsigned long currentMicros = 0;

int displaySize = 60;
float MaxBrightness = .5;
int OkGo = 0;
int MaxLoudness = 1;
float Threshold = .05;
int FadeStep = 10;
int NoiseFloor;
int LowBand = 1;
int MidBand = 4;

int RASamples = BUFFERSIZE;        // put whatever is here into the below
uint8_t LowSamples[BUFFERSIZE];
uint8_t MidSamples[BUFFERSIZE];
uint8_t AvgLowRALong[BUFFERSIZE];
uint8_t AvgMidRALong[BUFFERSIZE];
// eventually replace the above with the below:
//int LowSamplesArray[BUFFERSIZE];
//int MidSamplesArray[BUFFERSIZE];
//int AvgLowSamplesArray[BUFFERSIZE];
//int AvgMidSamplesArray[BUFFERSIZE];
byte getPos = 0;
byte putPos = 0;
byte nextPos = 0;
bool OKFull = 0;

int samples = 0;
uint8_t FirstRun = 0;


void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    #endif
    
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

//    TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc0
    DIDR0 = 0x01; // turn off the digital input for adc0

    doInit();
}

void doInit() {
    #ifdef DEBUG
    Serial.println("doInit()");
    #endif
    
    sampleInput();
    sampleFix();
    
    byte zNoise = 0;
    for (int i=1; i < FHT_N/2; i++) {
        zNoise = SampleL[i] + zNoise;
    }
    NoiseFloor = zNoise/sizeof(SampleL);
    
    colorWipe(strip.Color((MaxBrightness * 255), 0, 0), 1);
    colorWipe(strip.Color(0, (MaxBrightness * 255), 0), 1);
    colorWipe(strip.Color(0, 0, (MaxBrightness * 255)), 1);
}

void loop() {

    sampleInput();
    sampleFix();
    
    doAnalyze(); 
//    doFade();

//      drawSpectrum();
}


void doFade() {
    #ifdef DEBUG
    Serial.println("doFade()");
    #endif
    
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
//    delay(1);
}

void doAnalyze() {
    #ifdef DEBUG
    Serial.println("doAnalyze");
    #endif
    
    float zVal;
    uint8_t zLowMax = 0;
    uint16_t zLowTotal = 0;
    float zLowAvg = 0;
    uint8_t zPrevLow = 0;
//    if (FirstRun < BUFFERSIZE) {
    
    if (OKFull != 1) {
        for (int i=0; i < putPos; i++) {
            zLowTotal += LowSamples[i];
            if (LowSamples[i] > zLowMax) {
                zLowMax = LowSamples[i];
            }
        }
        zLowAvg = zLowTotal/putPos;
    }
    else {
        for (int i=0; i < BUFFERSIZE; i++) {
            zLowTotal += LowSamples[i];
            if (LowSamples[i] > zLowMax) {
                zLowMax = LowSamples[i];
            }
        }
        zLowAvg = zLowTotal/BUFFERSIZE;
    }
    currentMicros = micros();
    
    float zLowAvgNorm = zLowAvg/255.;
    float zLowMaxNorm = float(zLowMax)/255.;
    float zLowVal = ((1-zLowMaxNorm)/(1-zLowAvgNorm));
//    float zLowVal = (zLowMax-zLowAvg)/255.;

// how to detect a beat??

//// LEFT OFF HERE
    uint8_t tempPos = 0;
    if (putPos == 0) tempPos = BUFFERSIZE-1;
    else tempPos = putPos-1;
//    if (tempPos == 0) tempPos2 = BUFFURSIZE-1;
//    else tempPos2 = tempPos-1;
//    zPrevLow = (LowSamples[tempPos] + LowSamples[tempPos2])/2;
    zPrevLow = LowSamples[tempPos];
    zLowAttackSharpness = SampleL[LowBand] - zPrevLow;
    zLowAttackSharpness = (zLowAttackSharpness * .2) + zLowAttackSharpness;
    if (SampleL[LowBand] > zLowAvg
    LowSamples
(zLowMax - Threshold*255)
zLowAvgNorm
zLowSamples[tempPos]

    if (SampleL[LowBand] > 200 && (currentMicros - previousMicros) > interval)
    {
        drawSpectrum();
        previousMicros = currentMicros;
    }
    else {
        doFade();
    }    
    
//    if (SampleL[LowBand] > (zLowMax - int(255. * Threshold)))
//    {
//        drawSpectrum();
//    }
//    else {
//        doFade();
//    }
//    if (SampleL[LowBand] < Threshold && zLowVal > 0) drawSpectrum();
    
    #ifdef DEBUG
    Serial.print("\tzLowMax: ");
    Serial.print(zLowMax);
    Serial.print("\tzLowTotal: ");
    Serial.print(zLowTotal);
    Serial.print("\tzLowAvg: ");
    Serial.print(zLowAvg);
    Serial.print("\tzLowVal: ");
    Serial.println(zLowVal);
    #endif
    
//    if (SampleL[LowBand] > 150) {
////        Serial.println("\tyes, do drawSpectrum");
//        drawSpectrum();
//    }

//    float zValNorm = float(SampleL[LowBand])/255.;
//    float zAvgNorm = zAvg/255.;
//    zVal = (1-zValNorm)/(1-zAvgNorm);
//    if (zVal < Threshold && zVal > 0) drawSpectrum();
    
//
//    #ifdef DEBUG
//    Serial.println("");
//    Serial.print("\tsize: ");
//    Serial.print(sizeof(AvgLowRALong));
//    Serial.print("\tzTotal: ");
//    Serial.print(zTotal);
//    Serial.print("\tzMax: ");
//    Serial.print(zMax);
//    Serial.print("\t\tzAvg: ");
//    Serial.println(zAvg);
//
//    Serial.print("\tSampleL[LowBand]: ");
//    Serial.print(SampleL[LowBand]);
//    Serial.print("\tzValNorm: ");
//    Serial.print(zValNorm);
//    Serial.print("\tzAvgNorm: ");
//    Serial.print(zAvgNorm);
//    Serial.print("\tzVal: ");
//    Serial.println(zTempVal);
//    #endif
}

void sampleInput() {
    #ifdef DEBUG
    Serial.println("sampleInput()");
    #endif
    
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
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_lin();
    sei();
}

void sampleFix() {
    #ifdef DEBUG
    Serial.println("sampleFix()");
    #endif
    
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
    
    LowSamples[putPos] = SampleL[LowBand];         // somewhere around 100 hz
    MidSamples[putPos] = SampleL[MidBand];         // somewhere around 1000 hz
    putPos = (putPos + 1) & (BUFFERSIZE-1);
    if (putPos > BUFFERSIZE-1)putPos = BUFFERSIZE-1;
    if (putPos < 0) putPos = 0;
    
    
    #ifdef DEBUG
    Serial.print("\tSampleL[LowBand]: ");
    Serial.print(SampleL[LowBand]);
    Serial.println("");
    Serial.print("\tBUFFERSIZE: ");
    Serial.print(BUFFERSIZE);
    Serial.print("\tputPos: ");
    Serial.println(putPos);
    #endif

/////////////////////////////////////////////////////////////////
///////////// CIRCULAR ARRAY THINGY //////////////////////////
/////////////////////////////////////////////////////////////////

    
//    // getcount is how many entries, getsize is length of array
//    Serial.print("\tLowSamples: (");
//    for (int i=0; i < LowSamples.getSize(); i++) {
//        Serial.print(LowSamples.getElement(i));
//        Serial.print(",");
//    }
//    Serial.println("");
//    
//    if (samples % RASamples == 0) {
//        AvgLowRALong.addValue(LowSamples.getAverage());
//        AvgMidRALong.addValue(MidSamples.getAverage());
//    }
    // getcount is how many entries, getsize is length of array
    #ifdef DEBUG
    Serial.print("\tLowSamples: (");
    for (int i=0; i < sizeof(LowSamples); i++) {
        Serial.print(LowSamples[i]);
        Serial.print(",");
    }
    Serial.println("");
    #endif
    
//    if (samples % RASamples == 0) {
//        AvgLowRALong.addValue(LowSamples.getAverage());
//        AvgMidRALong.addValue(MidSamples.getAverage());
//    }


//    Serial.print("\tSampleL: (");
//    for (int i = 0 ; i < FHT_N/2 ; i++) {
//        Serial.print(SampleL[i]);
//        Serial.print(",");
//    }
//    Serial.println(")");
//
////////////////////////////////////////////////////////////////////////////////////////////
//    uint8_t zVal;
//    uint8_t zMax = 0;
//    uint16_t zTotal = 0;
//
//    for (int i=0; i < BUFFERSIZE; i++) {
//        zTotal += LowSamples[i];
//        if (LowSamples[i] > zMax) {
//            zMax = LowSamples[i];
//        }
//        zTotal = zTotal + AvgLowRALong[i];
//    }
//    
//    uint8_t zAvg = zTotal/sizeof(AvgLowRALong);
////    int zAvg = AvgLowRALong.getAverage();
//
////    #ifdef DEBUG
////    Serial.println("");
////    Serial.print("\tsize: ");
////    Serial.print(sizeof(AvgLowRALong));
////    Serial.print("\tzTotal: ");
////    Serial.print(zTotal);
////    Serial.print("\tzMax: ");
////    Serial.print(zMax);
////    Serial.print("\t\tzAvg: ");
////    Serial.println(zAvg);
////    #endif
//    
////    zVal = map(SampleL[LowBand], AvgLowRALong.getAverage(), AvgLowRALong.getMax(), 0, 100);
////    Serial.print(int(SampleL[LowBand]));
//    float zValNorm = float(SampleL[LowBand])/255.;
//    float zAvgNorm = float(zAvg)/255.;
//    float zTempVal = (1-zValNorm)/(1-zAvgNorm);
//    if (zTempVal < Threshold && zTempVal > 0) drawSpectrum();
//
//    zVal = zValNorm/zAvgNorm;
//    
//    #ifdef DEBUG
//    Serial.print("\tSampleL[LowBand]: ");
//    Serial.print(SampleL[LowBand]);
//    Serial.print("\tzValNorm: ");
//    Serial.print(zValNorm);
//    Serial.print("\tzAvgNorm: ");
//    Serial.print(zAvgNorm);
//    Serial.print("\tzVal: ");
//    Serial.println(zTempVal);
//    #endif
/////////////////////////////////////////////////////////////////////////////////

}  
  

void ISRBassWait () {
    #ifdef DEBUG
    Serial.println("ISRBassWait()");
    #endif
    
    for ( int i = 0 ; i < 60 ; i++ ) {
        uint32_t zcolor = strip.getPixelColor(i);
    }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}


//      SampleL[x] = tempY/256*MaxBrightness;
   
//     if ( tempY > 1 ) { SampleL[x] = ((tempY/(FHT_N/2))*100); }    // single channel full 16 LED high
//     else { SampleL[x] = 0; }
////    } else { sampleR[x] = ((tempY/256)*16); }    // single channel full 16 LED high


//  Serial.println("SampleL[x]");
//  Serial.println(sampleSet);
//for ( int i=1; i < displaySize; i++ ) {
//  if ( i == displaySize-1 ) {
//    Serial.println(SampleL[i]);
//  }
//  else {
//  Serial.print(SampleL[i]);
//  Serial.print(", ");
//  }
//}

