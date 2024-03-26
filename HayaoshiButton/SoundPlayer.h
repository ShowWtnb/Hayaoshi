#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

typedef enum sound_type
{
  SOUND_BUZZER = 0,
  SOUND_CORRECT,
  SOUND_WRONG,

  SOUND_NONE = -1
} SoundType;

void v_playSound(const unsigned char sound[], int len);
void playSound(SoundType soundType);
void asyncPlaySound(SoundType soundType);
void v_asyncPlaySound(const unsigned char sound[], int len);
void handlePlaySound();

#endif // SOUND_PLAYER_H
