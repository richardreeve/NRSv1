// $Id: song.h,v 1.3 2004/05/11 14:43:51 richardr Exp $
/*
 * Cricket song generator header file
 * by Richard Reeve
 */

#define MAX_AMP 32767
#define MAX_BUFFER 500000
#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define AUDIO_CHANNEL "/dev/dsp"

typedef enum DeviceType
{
  DeviceAudio,       /// Output directly to soundcard
  DeviceWav,         /// Output to .wav file
  DeviceUnknown      /// unknown - output raw
} DeviceType;

typedef struct SongTarget
{
  char *iName;             /// name of target
  int iFD;                 /// File descriptor for output
  DeviceType iDeviceType;  /// What type of output
  int iNumChannels;        /// Number of channels
  int iSamplingRate;       /// sampling rate in Hz
  int iSongLength;         /// total length of song:
  ///                           in ms if positive
  ///                           continuous if zero
  ///                           in chirps if negative
  int iMaxBuffer;          /// maximum buffer length
} SongTarget;

typedef struct SongStructure
{
  int iCarrierFrequency; /// carrier frequency in Hz
  int iStartingOffset;   /// pause before song start in ms
  double iMaxAmplitude;  /// proportion of maximum volume [0,1]
  int iAttack;           /// attack of syllable in ms
  int iSustain;          /// sustain of syllable in ms
  int iDecay;            /// decay of syllable in ms
  int iRecovery;         /// recovery before next syllable in ms
  int iNumSyllables;     /// number of syllables per chirp
  int iChirpRefract;     /// gap between chirps in ms 
  ///                         (actual gap is iChirpRefract + iRecovery)
  int iIsSimple;         /// Is it a simple structure like normal, or 
  ///                         something strange?
  /// If the structure is strange, then use iVar..., not i...
  int *iVarAttack;       /// attack of each syllable in ms
  int *iVarSustain;      /// sustain of each syllable in ms
  int *iVarDecay;        /// decay of each syllable in ms
  int *iVarRecovery;     /// recovery before next syllable in ms
} SongStructure;

typedef struct Song
{
  SongTarget *iSongTarget;      /// Target for song
  SongStructure iSongStructure; /// Copy of SongStructure
  double iSinScale;             /// carrier frequency in radians per sample
  short iMaxNum;                /// proportion of maximum volume [0,MAX_AMP]
  int iOffsetNum;               /// pause before start of song in samples
  int iAttackNum;               /// attack of syllable in samples
  int iSustainNum;              /// sustain of syllable in samples
  int iDecayNum;                /// decay of syllable in samples
  int iRecoveryNum;             /// recovery before next syllable in samples
  int iNumSyllables;            /// number of syllables per chirp
  int iChirpRefractNum;         /// gap between chirps in samples
  int iNumChirps;               /// number of chirps in song (0 is infinity)
  int iIsSimple;                /// Is it a simple structure like normal, or 
  ///                                something strange?
  /// If the structure is strange, then use iVar...Num, not i...Num
  int *iVarAttackNum;           /// attack of each syllable in ms
  int *iVarSustainNum;          /// sustain of each syllable in ms
  int *iVarDecayNum;            /// decay of each syllable in ms
  int *iVarRecoveryNum;         /// recovery before next syllable in ms
  short *iBuffer;               /// buffer containing song
  short *iBuffPtr;              /// pointer to current position in buffer
  int iAllocated;               /// allocated size of buffer
  int iBuffNum;                 /// number of samples in buffer
  int iBuffChirps;              /// number of chirps to be put into buffer
  int iRepetitions;             /// number of repetitions of full buffer
  int iLastNum;                 /// number of samples to take after last rep.
  // for information only
  int iSyllLen;                 /// length of syllable in samples
  int iISI;                     /// inter syllable interval in samples
  int *iVarSyllLen;
  int *iVarISI;
  int iChirpLen;                /// length of chirp in samples
  int iICI;                     /// inter chirp interval in samples
  double iScale;                 /// sample timeslice ( 1 / sample frequency )
} Song;

// Open new target for song
SongTarget *open_song_target( const char *aDevice, int channels, int rate );

// Change number of channels for target
int change_num_channels( SongTarget *aTarget );

// Change target sample rate
int change_sample_rate( SongTarget *aTarget );

// print out song target
void print_song_target( SongTarget *aTarget );

// close a song target and free memory
int close_target( SongTarget *aTarget );

// Get a copy of the default song structure
SongStructure get_default_song_structure();

// print out a song structure
void print_song_structure( SongStructure anSS );

// Create a song from a copy of a SongStructure directed at a target
Song *create_song( SongStructure anSS, SongTarget *aTarget );

// Send song to new target, return number of changes (0 if unchanged)
int retarget_song( Song *aSong, SongTarget *aTarget );

// Print out a song and its associated target and structure
void print_song( Song *aSong );

// Get next buffer of a 1 channel song in songBuffer, return size of buffer.
// songBuffer must be of size iMaxBuffer * sizeof( short )
int get_next_song_buffer( Song *theSong, short *songBuffer, int newSong );

// Play a 1 channel song 
void play_song( Song *theSong );

// Get next buffer of a 2 channel song in songBuffer, return size of buffer.
// songBuffer must be of size iMaxBuffer * sizeof( short )
int get_next_songs_buffer( Song *theLeftSong, Song *theRightSong,
			   short *songBuffer, int newSong );

// Play a 2 channel song 
void play_songs( Song *theLeftSong, Song *theRightSong );

// Play buffer to target
void play_buffer( short *songBuffer, int size, SongTarget *aTarget );

// Destroy a song, free memory
void destroy_song( Song *theSong );
