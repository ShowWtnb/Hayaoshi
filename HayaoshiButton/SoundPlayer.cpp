#include "Arduino.h"
#include "SoundPlayer.h"
#include "my_sound.h"

// 音声を再生する
void v_playSound(const unsigned char sound[], int len)
{
    // Serial.println("play sound");
    for (int i = 0; i < len; i++)
    {
        OCR2B = pgm_read_byte_near(&sound[i]);
        delayMicroseconds(100);
    }
    // OCR2B = pgm_read_byte_near(0x00);
    // Serial.println("play sound end");
}

void playSound(SoundType soundType)
{
    switch (soundType)
    {
    case SOUND_BUZZER:
        v_playSound(sound_buzzer, sound_buzzer_len);
        break;
    case SOUND_CORRECT:
        v_playSound(sound_correct, sound_correct_len);
        break;
    case SOUND_WRONG:
        v_playSound(sound_wrong, sound_wrong_len);
        break;

    default:
        break;
    }
}

void asyncPlaySound(SoundType soundType)
{
    switch (soundType)
    {
    case SOUND_BUZZER:
        v_asyncPlaySound(sound_buzzer, sound_buzzer_len);
        break;
    case SOUND_CORRECT:
        v_asyncPlaySound(sound_correct, sound_correct_len);
        break;
    case SOUND_WRONG:
        v_asyncPlaySound(sound_wrong, sound_wrong_len);
        break;

    default:
        break;
    }
}

unsigned char* soundPlaying;
int lenSoundPlaying;
int idxSoundPlaying= 0;
unsigned long current = -1;

void v_asyncPlaySound(const unsigned char sound[], int len)
{
    soundPlaying = sound;
    lenSoundPlaying = len;
    idxSoundPlaying= 0;
    
    Serial.println("Start Play Sound");
    OCR2B = pgm_read_byte_near(&soundPlaying[0]);
    current = micros();
    // delayMicroseconds(100);
    // for (int i = 0; i < len; i++)
    // {
    //     OCR2B = pgm_read_byte_near(&sound[i]);
    //     delayMicroseconds(100);
    // }

}

void handlePlaySound(){
    if (((micros() - current) > 100) && idxSoundPlaying < lenSoundPlaying )
    {
        // Serial.print("Playing ");
        // Serial.println(idxSoundPlaying);

        OCR2B = pgm_read_byte_near(&soundPlaying[idxSoundPlaying++]);
        current = micros();
    }    
}

