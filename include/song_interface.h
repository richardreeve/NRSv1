// $Id: song_interface.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
/**
 * Interface for song
 * Richard Reeve
 */

#define PORT_SONG 1172

typedef enum SongCommands
{
  SongInitialise,
  SongSetNumChannels,
  SongSetTarget,
  SongSetAttack,
  SongSetSustain,
  SongSetDecay,
  SongSetRecovery,
  SongSetNumSyllables,
  SongSetChirpRefractoryPeriod,
  SongSetOffset,
  SongSetGain,
  SongSetFrequency,
  SongSetDuration,
  SongSetNumChirps,
  SongSetContinuous,
  SongSetSamplingRate,
  SongGetNumChannels,
  SongGetTarget,
  SongGetAttack,
  SongGetSustain,
  SongGetDecay,
  SongGetRecovery,
  SongGetNumSyllables,
  SongGetChirpRefractoryPeriod,
  SongGetOffset,
  SongGetGain,
  SongGetFrequency,
  SongGetDuration,
  SongGetSamplingRate,
  SongRun,
  SongPause,
  SongResume,
  SongStop,
  SongQuit
} SongCommand;
