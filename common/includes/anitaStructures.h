/*! \file anitaStructures.h
  \brief Contains the definitions of all the structures used to store 
  and move ANITA data and stuff.
    
  Some simple definitions of the structures that we'll use to store 
  ANITA data. In here there will be the structures used for transient data,
  housekeeping stuff, GPS stuff and anything else that is going to be used
  by more than one of the processes. This will all change to use bit fields,
  at some point in the future. For now this is just a place holder we can
  test data flow with.

  August 2004  rjn@mps.ohio-state.edu
*/
#ifndef ANITA_STRUCTURES_H
#define ANITA_STRUCTURES_H
#include "includes/anitaFlight.h"

#define MAX_SATS 12

typedef enum {
    PACKET_HD = 0x100,
    PACKET_WV = 0x101,
    PACKET_SURF = 0x102,
    PACKET_SURF_HK = 0x103,
    PACKET_GPS_ADU5_PAT = 0x200,
    PACKET_GPS_ADU5_SAT = 0x201,
    PACKET_GPS_ADU5_VTG = 0x202,
    PACKET_GPS_G12_POS = 0x203,
    PACKET_GPS_G12_SAT = 0x204,
    PACKET_HKD = 0x300,
    PACKET_CMD_ECHO = 0x400,
    PACKET_MONITOR = 0x500
} PacketCode_t;

typedef struct {
    PacketCode_t code;    
    unsigned short numBytes;
    unsigned short badCrc;  //Willmaybe abandon for unique value  
} GenericHeader_t;

typedef struct {
    long unixTime;
    long unixTimeUs;
} TimeHeader_t;

// TURFIO data structure, TV test
/* typedef struct { */
/*     unsigned short rawtrig; // raw trigger number */
/*     unsigned short L3Atrig; // same as raw trig */
/*     unsigned long pps1;     // 1PPS */
/*     unsigned long time;     // trig time */
/*     unsigned long interval; // interval since last trig */
/*     unsigned int rate[2][4]; // Antenna trigger output rates (Hz) */
/* } TurfioStruct_t; */

/* typedef struct { */
/*     unsigned char otherBits; */
/*     unsigned char trigType; */
/*     unsigned char trigNumByte1; */
/*     unsigned char trigNumByte2; */
/*     unsigned char trigNumByte3; */
/*     unsigned char trigTimeByte1; */
/*     unsigned char trigTimeByte2; */
/*     unsigned char trigTimeByte3; */
/*     unsigned char trigTimeByte4; */
/*     unsigned char ppsNumByte1; */
/*     unsigned char ppsNumByte2; */
/*     unsigned char ppsNumByte3; */
/*     unsigned char ppsNumByte4; */
/*     unsigned char c3poByte1; */
/*     unsigned char c3poByte2; */
/*     unsigned char c3poByte3; */
/*     unsigned char c3poByte4; */
/* } TurfioStruct_t; */
    
  
//typedef struct {
//  unsigned char otherBits;
//    unsigned char trigType;
//    unsigned long trigNum;
//    unsigned long trigTime;
//    unsigned long ppsNum;
//    unsigned long c3poNum;
//} TurfioStruct_t;
  
typedef struct {
    unsigned short rawTrig; // raw trigger number
    unsigned short l3ATrig; // same as raw trig
    unsigned long ppsNum;     // 1PPS
    unsigned long trigTime;     // trig time
    unsigned long trigInterval; // interval since last trig
    unsigned int l1Rate[40][4]; // Antenna trigger output rates (Hz)
    unsigned int l2Rate[2][16]; // level 2 trigger rate
    unsigned int l3Rate[4][16];
    unsigned int vetoMon[16];
} TurfioStruct_t;
    
  

typedef struct {
    unsigned char chanId;   // chan+9*surf
    unsigned char chipIdFlag; // Bits 0,1 chipNum; Bit 3 hitBus wrap; 4-7 hitBusOff
    unsigned char firstHitbus;
    unsigned char lastHitbus;
} RawSurfChannelHeader_t;


typedef struct {
    RawSurfChannelHeader_t header;
    unsigned short data[MAX_NUMBER_SAMPLES];
} SurfChannelFull_t;


typedef struct {
    GenericHeader_t gHdr;
    long unixTime;       /* unix UTC sec*/
    long unixTimeUs;     /* unix UTC microsec */
    int gpsSubTime;     /* the GPS fraction of second (in ns) (for the X events per second that gets tagged with it */
    int eventNumber;    /* Global event number */
    short surfMask;
    short numChannels;  /* In case we turn part of array off, or whatever. */
    short numSamples;   /* total number of samples per waveform == 256*/
    char calibStatus;   /* Were we flashing the pulser? */
    char priority;
    TurfioStruct_t turfio; /*The 32 byte TURFIO data*/
} AnitaEventHeader_t;

typedef struct {
    SurfChannelFull_t channel[NUM_DIGITZED_CHANNELS];
} AnitaEventBody_t;

/* these are syntactic sugar to hel us keep track of 
typedef int Fixed3_t; /*rescaled integer left shifted 3 bits */
typedef int Fixed6_t; /*rescaled integer left shifted 6 bits */
typedef int Fixed8_t; /*rescaled integer left shifted 8 bits */

typedef struct {
     Fixed3_t ch[NUM_DIGITZED_CHANNELS]; 
     unsigned short valid_channels;
} AnitaTransientBody3_t; /* final corrected transient type 
			    used to calculate power */

typedef struct {
     Fixed6_t ch[NUM_DIGITZED_CHANNELS]; 
     unsigned short valid_channels;
} AnitaPowerBody6_t; /* power from squaring an AnitaTransientBody3 */

typedef struct {
     Fixed8_t ch[NUM_DIGITZED_CHANNELS]; 
     unsigned short valid_channels;
} AnitaTransientBody8_t; /* used for pedestal subtraction, unwrapping, 
			    averaging, and gain correction */


typedef struct {
    AnitaEventHeader_t header;
    AnitaEventBody_t body;
} AnitaEventFull_t;

typedef struct {
    GenericHeader_t gHdr;
    int eventNumber;
    SurfChannelFull_t waveform;
} WaveformPacket_t;

typedef struct {
    GenericHeader_t gHdr;
    int eventNumber;
    SurfChannelFull_t waveform[CHANNELS_PER_SURF];
} SurfPacket_t;

typedef struct {
    long unixTime;
    int subTime;
} GpsSubTime_t;

typedef struct {
    GenericHeader_t gHdr;
    long unixTime;
    long unixTimeUs;
    long timeOfDay;
    float heading;
    float pitch;
    float roll;
    float mrms;
    float brms;
    int attFlag;
    float latitude;
    float longitude;
    float altitude;
} GpsAdu5PatStruct_t;

typedef struct {
    char prn;
    char elevation;
    short azimuth;
    char snr;
    char flag; 
} GpsSatInfo_t;

typedef struct {
    GenericHeader_t gHdr;
    long unixTime;
    int numSats;
    GpsSatInfo_t sat[MAX_SATS];
} GpsG12SatStruct_t;

typedef struct {
    GenericHeader_t gHdr;
    long unixTime;
    int numSats[4];
    GpsSatInfo_t sat[4][MAX_SATS];
} GpsAdu5SatStruct_t;

typedef struct {
    GenericHeader_t gHdr;
    long unixTime;
    long unixTimeUs;
    float trueCourse;
    float magneticCourse;
    float speedInKnots;
    float speedInKPH;
} GpsAdu5VtgStruct_t;

typedef struct {
    GenericHeader_t gHdr;
    long unixTime;
    long unixTimeUs;
    long timeOfDay;
    int numSats;
    float latitude;
    float longitude;
    float altitude;
    float trueCourse;
    float verticalVelocity;
    float speedInKnots;
    float pdop;
    float hdop;
    float vdop;
    float tdop;
} GpsG12PosStruct_t;

typedef enum {
    IP320_RAW=0x100,
    IP320_AVZ=0x200,
    IP320_CAL=0x300
} AnalogueCode_t;

typedef struct {
    long unixTime;
    char status;
} CalibStruct_t;

typedef struct {
    unsigned short data[CHANS_PER_IP320];
} AnalogueDataStruct_t;

typedef struct {
    long data[CHANS_PER_IP320];
} AnalogueCorrectedDataStruct_t;

typedef struct {
    AnalogueCode_t code;
    AnalogueDataStruct_t board[NUM_IP320_BOARDS];
} FullAnalogueStruct_t;

typedef struct {
    int temp[2];
} SBSTemperatureDataStruct_t;

typedef struct {
    double x;
    double y;
    double z;
} MagnetometerDataStruct_t;

typedef struct {    
    GenericHeader_t gHdr;
    long unixTime;
    long unixTimeUs;
    FullAnalogueStruct_t ip320;
    MagnetometerDataStruct_t mag;
    SBSTemperatureDataStruct_t sbs;
} HkDataStruct_t;

typedef struct {    
    unsigned short threshold;
    unsigned short scaler[ACTIVE_SURFS][SCALERS_PER_SURF];
} SimpleScalerStruct_t;

typedef struct {    
    unsigned short globalThreshold; //set to zero if there isn't one
    unsigned short errorFlag; //Will define at some point    
    unsigned short upperWords[ACTIVE_SURFS];
    unsigned short scaler[ACTIVE_SURFS][SCALERS_PER_SURF];
    unsigned short threshold[ACTIVE_SURFS][SCALERS_PER_SURF];
    unsigned short rfPower[ACTIVE_SURFS][RFCHAN_PER_SURF];
} FullSurfHkStruct_t;


typedef struct {
    GenericHeader_t gHdr;
    long unixTime;
    char goodFlag; // 0 is bad, 1 is good
    char numCmdBytes; // number of cmd bytes (upto 10)
    char cmd[10]; // the cmd bytes
} CommandEcho_t;

typedef enum {
    PRI_FORCED = 0,
    PRI_CALIB = 1,
    PRI_1 = 1,
    PRI_2,
    PRI_3,
    PRI_4,
    PRI_TIMEOUT,
    PRI_6, 
    PRI_7,
    PRI_8,
    PRI_PAYLOAD
} PriorityCode;

typedef struct {
    unsigned short mainDisk; //In MegaBytes
    unsigned short usbDisk[NUM_USBDISKS]; //In MegaBytes
} DiskSpaceStruct_t;

typedef struct {
    unsigned short eventLinks[NUM_PRIORITIES]; //10 Priorities
    unsigned short cmdLinks;
    unsigned short gpsLinks;
    unsigned short hkLinks;
    unsigned short monitorLinks;
} QueueStruct_t;

typedef struct {
    GenericHeader_t gHdr;
    long unixTime;
    DiskSpaceStruct_t diskInfo;
    QueueStruct_t queueInfo;
} MonitorStruct_t;

typedef struct {
     long unixTime; /* when were these taken? */
     int nsamples; /* how many samples in the average? */
     float thePeds[ACTIVE_SURFS][LABRADORS_PER_SURF]
     [CHANNELS_PER_SURF][MAX_NUMBER_SAMPLES]; /* mean pedestal */
     float pedsRMS[ACTIVE_SURFS][LABRADORS_PER_SURF]
     [CHANNELS_PER_SURF][MAX_NUMBER_SAMPLES]; 
               /* RMS of the samples (not of mean */
} PedestalStruct_t;

#endif /* ANITA_STRUCTURES_H */
