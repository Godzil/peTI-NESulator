/*
 *  Allegro Sound Driver for EMULib Sound system - The TI-NESulator Project
 *  SndAlleg.C
 *
 *  Created by Manoel Trapier
 *  Copyright 2003-2008 986 Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */
#include <Sound.h>

/* Allegro includes */
#ifdef __APPLE__
#define USE_CONSOLE
#include <Allegro/allegro.h>
#else
#define USE_CONSOLE
#include <allegro.h>
#endif

#include <os_dependent.h>

#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <pthread.h>
//#include <sys/ioctl.h>
    
#define AUDIO_CONV(A) (128+(A))


AUDIOSTREAM *stream;

static pthread_t ThreadID;
static int SoundRate    = 0;
static int MasterVolume = 64;
static int MasterSwitch = (1<<SND_CHANNELS)-1;
static int LoopFreq     = 25;
static int NoiseGen     = 1;
static int Suspended    = 0;

static struct
{
  int Type;                       /* Channel type (SND_*)             */
  int Freq;                       /* Channel frequency (Hz)           */
  int Volume;                     /* Channel volume (0..255)          */

  const signed char *Data;              /* Wave data (-128..127 each)       */
  int Length;                     /* Wave length in Data              */
  int Rate;                       /* Wave playback rate (or 0Hz)      */
  int Pos;                        /* Wave current position in Data    */  

  int Count;                      /* Phase counter                    */
} CH[SND_CHANNELS];

static void UnixSetWave(int Channel,const signed char *Data,int Length,int Rate);
static void UnixSetSound(int Channel,int NewType);
static void UnixDrum(int Type,int Force);
static void UnixSetChannels(int Volume,int Switch);
static void UnixSound(int Channel,int NewFreq,int NewVolume);

static int  OpenSoundDevice(int Rate,int Verbose);
static void *DSPLoop(void *Arg);

/** StopSound() **********************************************/
/** Temporarily suspend sound.                              **/
/*************************************************************/
void StopSound(void) { Suspended=1; }

/** ResumeSound() ********************************************/
/** Resume sound after StopSound().                         **/
/*************************************************************/
void ResumeSound(void) { Suspended=0; }

/** OpenSoundDevice() ****************************************/
/** Open /dev/dsp with a given level of sound quality.      **/
/** Returns 0 if failed or sound quality (Mode).            **/
/*************************************************************/
static int OpenSoundDevice(int Rate,int Verbose)
{
  voice_start(stream->voice);
  
  if(Verbose) puts("OK");
  return(Rate);
}

/** DSPLoop() ************************************************/
/** Main loop of the sound server.                          **/
/*************************************************************/
static void *DSPLoop(void *Arg)
{
  int Wave[SND_BUFSIZE];
  unsigned char *Buf;
  register int J,I,K,L,M,N,L1,L2,A1,A2,V;
  int FreqCount;
   N = L = A2 = 0;

  for(J=0;J<SND_CHANNELS;J++)
  {
    CH[J].Type   = SND_MELODIC;
    CH[J].Count  = 0;
    CH[J].Volume = 0;
    CH[J].Freq   = 0;
  }



  FreqCount=SoundRate/SND_BUFSIZE;

  for(;;)
  {
    Buf = get_audio_stream_buffer(stream);

      if (Buf) {
      FreqCount-=LoopFreq;
      
    /* If suspending sound... */
    if(Suspended)
    {
      /* Close sound device */
      while(Suspended) sleep(1);
      /* Reopen sound device */
      SoundRate=OpenSoundDevice(SoundRate,0);
    }

    /* Waveform generator */
    for(J=0,M=MasterSwitch;M&&(J<SND_CHANNELS);J++,M>>=1)
      if(CH[J].Freq&&(V=CH[J].Volume)&&(M&1))
        switch(CH[J].Type)
        {
          case SND_NOISE: /* White Noise */
            /* For high frequencies, recompute volume */
            if(CH[J].Freq<=SoundRate) K=0x10000*CH[J].Freq/SoundRate;
            else { V=V*SoundRate/CH[J].Freq;K=0x10000; }
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L1+=K;
              if(L1&0xFFFF0000)
              {
                L1&=0xFFFF;
                if((NoiseGen<<=1)&0x80000000) NoiseGen^=0x08000001;
              }
              Wave[I]+=NoiseGen&1? V:-V;
            }
            CH[J].Count=L1;
            break;

          case SND_WAVE: /* Custom Waveform */
            /* Waveform data must have correct length! */
            if(CH[J].Length<=0) break;
            /* Start counting */
            K  = CH[J].Rate>0? (SoundRate<<15)/CH[J].Freq/CH[J].Rate
                             : (SoundRate<<15)/CH[J].Freq/CH[J].Length;
            L1 = CH[J].Pos%CH[J].Length;
            L2 = CH[J].Count;
            A1 = CH[J].Data[L1]*V;
            /* If expecting interpolation... */
            if(L2<K)
            {
              /* Compute interpolation parameters */
              A2 = CH[J].Data[(L1+1)%CH[J].Length]*V;
              L  = (L2>>15)+1;
              N  = ((K-(L2&0x7FFF))>>15)+1;
            }
            /* Add waveform to the buffer */
            for(I=0;I<SND_BUFSIZE;I++)
              if(L2<K)
              {
                /* Interpolate linearly */
                Wave[I]+=A1+L*(A2-A1)/N;
                /* Next waveform step */
                L2+=0x8000;
                /* Next interpolation step */
                L++;
              }
              else
              {
                L1 = (L1+L2/K)%CH[J].Length;
                L2 = (L2%K)+0x8000;
                A1 = CH[J].Data[L1]*V;
                Wave[I]+=A1;
                /* If expecting interpolation... */
                if(L2<K)
                {
                  /* Compute interpolation parameters */
                  A2 = CH[J].Data[(L1+1)%CH[J].Length]*V;
                  L  = 1;
                  N  = ((K-L2)>>15)+1;
                }
              }
            /* End counting */
            CH[J].Pos   = L1;
            CH[J].Count = L2;
            break;
 
 
          case SND_QS_DU0:
             /* Do not allow frequencies that are too high */
            if(CH[J].Freq>=SoundRate/3) break;
            K=0x10000*CH[J].Freq/SoundRate;
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L2=L1+K;
              Wave[I]+=L1&0x2000?(L2&0x8000? V:0):(L2&0x8000? 0:-V);
              L1=L2;
            }
            CH[J].Count=L1;
            break;
            
          case SND_QS_DU1:
       /* Do not allow frequencies that are too high */
            if(CH[J].Freq>=SoundRate/3) break;
            K=0x10000*CH[J].Freq/SoundRate;
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L2=L1+K;
              Wave[I]+=L1&0x4000?(L2&0x8000? V:0):(L2&0x8000? 0:-V);
              L1=L2;
            }
            CH[J].Count=L1;
            break;
            
          case SND_QS_DU3:
       /* Do not allow frequencies that are too high */
            if(CH[J].Freq>=SoundRate/3) break;
            K=0x10000*CH[J].Freq/SoundRate;
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L2=L1+K;
              Wave[I]+=L1&0xC000?(L2&0x4000? V:0):(L2&0xC000? 0:-V);
              L1=L2;
            }
            CH[J].Count=L1;
            break;
            
          case SND_QS_DU2:
          case SND_MELODIC: /* Melodic Sound */         
          default:          /* Default Sound */
            /* Do not allow frequencies that are too high */
            if(CH[J].Freq>=SoundRate/3) break;
            K=0x10000*CH[J].Freq/SoundRate;
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L2=L1+K;
              Wave[I]+=L1&0x8000? (L2&0x8000? V:0):(L2&0x8000? 0:-V);
              L1=L2;
            }
            CH[J].Count=L1;
            break;
              
          case SND_TRIANGLE:          /* Default Sound */
            /* Do not allow frequencies that are too high */
            if(CH[J].Freq>=SoundRate/3) break;
            K=0x10000*CH[J].Freq/SoundRate;
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L2=L1+K;
              Wave[I]+= L1&0x8000?V:-V /*(L2&0x8000? V:0):(L2&0x8000? 0:-V)*/;
              L1=L2;
            
            
            }
            CH[J].Count=L1;
            break;
        }

    /* Mix and convert waveforms */
    for(J=0;J<SND_BUFSIZE;J++)
    {
      I=(Wave[J]*MasterVolume)>>16;
      I=I<-128? -128:I>127? 127:I;
      Buf[J]=AUDIO_CONV(I);
      Wave[J]=0;
    }
    free_audio_stream_buffer(stream);
    }
  }

  return(0);
}

/** InitSound() **********************************************/
/** Initialize DSP. Returns Rate on success, 0 otherwise.   **/
/** Mode is 0 to skip initialization (will be silent).      **/
/*************************************************************/
int InitSound(int Rate,int Verbose)
{
  /* If sound was initialized, kill it */
  TrashSound();

  /* Silence requested */
  if(Rate<=0) return(0);

  /* Synthesis rate should be at least 8kHz */
  if(Rate<8192) Rate=44100;

  /* Initialize things */
  SoundRate = 0;
  ThreadID  = 0;
  Suspended = 0;

  /* Set driver functions */
  SndDriver.SetSound    = UnixSetSound;
  SndDriver.Drum        = UnixDrum;
  SndDriver.SetChannels = UnixSetChannels;
  SndDriver.Sound       = UnixSound;
  SndDriver.SetWave     = UnixSetWave;

  if (install_sound(DIGI_AUTODETECT, MIDI_NONE, "") != 0)
  {
      console_printf(Console_Error, "%s!\n", allegro_error);
      return 1;
  }


  stream = play_audio_stream(SND_BUFSIZE, 8, FALSE, Rate, 255, 128);
  if (!stream) {
      console_printf(Console_Error, "Error creating audio stream!\n");
      return 1;
  }

  voice_stop(stream->voice);

  /* Open sound device */
  if(Verbose) puts("Starting sound server:");
  if(!(Rate=OpenSoundDevice(Rate,Verbose))) return(0);

  /* Create DSPLoop() thread */
  if(Verbose) console_printf(Console_Default, "  Creating thread...");
  if(pthread_create(&ThreadID,0,DSPLoop,0))
  { if(Verbose) puts("FAILED");return(0); }

  /* Detach the thread */
  pthread_detach(ThreadID);

  /* Done */
  if(Verbose) puts("OK");
  return(SoundRate=Rate);
}

/** TrashSound() *********************************************/
/** Shut DSP down.                                          **/
/*************************************************************/
void TrashSound(void)
{
  StopSound();
  console_printf(Console_Default, "%s: Kill thread...\n", __func__);
  if(ThreadID)    pthread_cancel(ThreadID);

  SoundRate = 0;
  ThreadID  = 0;
}

/** UnixSound() **********************************************/
/** Generate sound of given frequency (Hz) and volume       **/
/** (0..255) via given channel.                             **/
/*************************************************************/
void UnixSound(int Channel,int NewFreq,int NewVolume)
{
  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  if(!NewVolume||!NewFreq) { NewVolume=0;NewFreq=0; }

  CH[Channel].Volume = NewVolume;
  CH[Channel].Freq   = NewFreq;
}

/** UnixSetChannels() ****************************************/
/** Set master volume (0..255) and turn channels on/off.    **/
/** Each bit in Toggle corresponds to a channel (1=on).     **/
/*************************************************************/
void UnixSetChannels(int MVolume,int MSwitch)
{
  /* Set new MasterSwitch value */
  MasterSwitch = MSwitch;
  MasterVolume = MVolume;
}

/** UnixSetSound() *******************************************/
/** Set sound type (SND_NOISE/SND_MELODIC) for a given      **/
/** channel.                                                **/
/*************************************************************/
void UnixSetSound(int Channel,int NewType)
{
  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  CH[Channel].Type = NewType;
}


/** UnixSetWave() ********************************************/
/** Set waveform for a given channel. The channel will be   **/
/** marked with sound type SND_WAVE. Set Rate=0 if you want **/
/** waveform to be an instrument or set it to the waveform  **/
/** own playback rate.                                      **/
/*************************************************************/
void UnixSetWave(int Channel,const signed char *Data,int Length,int Rate)
{
  if((Channel<0)||(Channel>=SND_CHANNELS)||(Length<=0)) return;

  CH[Channel].Type   = SND_WAVE;
  CH[Channel].Length = Length;
  CH[Channel].Rate   = Rate;
  CH[Channel].Pos    = 0;
  CH[Channel].Count  = 0;
  CH[Channel].Data   = Data;
}

/** UnixDrum() ***********************************************/
/** Hit a drum of a given type with given force.            **/
/*************************************************************/
void UnixDrum(int Type,int Force)
{
  /* This function is currently empty */
}
