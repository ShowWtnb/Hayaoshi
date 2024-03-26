#include "main.h"
#include "my_sound.h"
#include "StatusLedController.h"
#include "PlayerLedController.h"
#include "SoundPlayer.h"

int PushedButtonIdx = -1;
Event ev = evNONE;
State st = stNONE;
Mode md = mdSINGLE_CHANCE;
// StatusLedController ledController(STATUS_LED_PIN);
StatusLedController *ledController;
PlayerLedController *playerLedController;

void setup()
{
  // put your setup code here, to run once:
  // Stateの初期化
  st = stTURN_ON;

  // デバッグ用出力の有効化
  Serial.begin(115200);
  while (!Serial)
  {
  }

  Serial.println("setup start");

  // ステータス表示用LEDの初期化
  ledController = new StatusLedController(STATUS_LED_PIN);

  // 各ボタンのLEDControllerの設定
  playerLedController = new PlayerLedController(SER_74HC595, RCLK_74HC595, SRCLK_74HC595);
  playerLedController->init();

  // 音の再生の初期設定
  initSoundOutput();

  if (digitalRead(WRONG_PIN) == LOW)
  {
    Serial.println("WRONG_PIN pushed while initialization");
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    Serial.println("Mode setted as ENDLESS_CHANCE");
    md = mdENDLESS_CHANCE;
  }
  // // デバッグ用
  // md = mdENDLESS_CHANCE;

  // 割り込みボタンの設定
  initInterruptButtonInterrupt();

  ev = evINIT_END;
}

void loop()
{
  switch (md)
  {
  case mdSINGLE_CHANCE:
    mainSingle();
    break;
  case mdENDLESS_CHANCE:
    mainEndless();
    break;
  case mdNONE:
  default:
    mainSingle();
    break;
  }
  ledController->handleEvent();
  playerLedController->handleEvent();
  handlePlaySound();
}

// シングルチャンスの動作
void mainSingle()
{
  switch (st)
  {
  case stTURN_ON:
    switch (ev)
    {
    case evINIT_END:
      ev = evNONE;
      Serial.println("setup end");
      ledController->onLed();
      st = stSTANDBY;
      break;
    default:
      break;
    }
  case stSTANDBY:
    switch (ev)
    {
    case evBUTTON_PUSHED:
      ev = evNONE;
      // 何番が押されたかチェックする
      for (size_t i = 0; i < IOpinCnt; i++)
      {
        if (digitalRead(ButtonPins[i]) == LOW)
        {
          PushedButtonIdx = i;
          Serial.print(PushedButtonIdx);
          Serial.println(" was pushed");
          break;
        }
      }

      if (PushedButtonIdx != -1)
      {
        // 対応するLEDを点灯する
        // LedON(PushedButtonIdx);
        playerLedController->onLed(PushedButtonIdx);
        playerLedController->onBlinkLed(PushedButtonIdx);
        // 音声を再生する
        // v_playSound(sound_buzzer, sound_buzzer_len);
        playSound(SOUND_BUZZER);
        // 状態表示用LEDを点滅状態にする
        ledController->setLedBlinkCount(PushedButtonIdx);
        ledController->startBlink();
        // 解答中状態に遷移する
        st = stANSWER;
      }
      break;
    default:
      break;
    }
    break;
  case stANSWER: // 解答中状態
    switch (ev)
    {
    case evBUTTON_PUSHED:
      ev = evNONE;

      // 押されたのがリセットボタンかチェックする
      if (digitalRead(CORRECT_PIN) == LOW)
      {
        PushedButtonIdx = -1;
        ev = evCORRECT_PUSHED;
        break;
      }
      if (digitalRead(WRONG_PIN) == LOW)
      {
        PushedButtonIdx = -1;
        ev = evWRONG_PUSHED;
        break;
      }
      // シングルチャンスなので他の解答者のボタンは押されていても無視する
      break;
    case evCORRECT_PUSHED:
      Serial.print("stANSWER");
      Serial.println(" evCORRECT_PUSHED");
      ev = evNONE;
      // LEDを消灯する
      // clearLeds();
      playerLedController->clearAllLeds();
      // 音を再生する
      playSound(SOUND_CORRECT);
      // v_playSound(sound_correct, sound_correct_len);
      // 点滅を止める
      ledController->stopBlink();
      // ボタン待機状態に戻る
      st = stSTANDBY;
      break;
    case evWRONG_PUSHED:
      Serial.print("stANSWER");
      Serial.println(" evWRONG_PUSHED");
      ev = evNONE;
      // LEDを消灯する
      // clearLeds();
      playerLedController->clearAllLeds();
      // 音を再生する
      // v_playSound(sound_wrong, sound_wrong_len);
      playSound(SOUND_WRONG);
      // 点滅を止める
      ledController->stopBlink();
      // ボタン待機状態に戻る
      st = stSTANDBY;
      break;

    default:
      break;
    }
    break;
  default:
    break;
  }
}
// エンドレチャンスの動作
void mainEndless()
{
  switch (st)
  {
  case stTURN_ON:
    switch (ev)
    {
    case evINIT_END:
      ev = evNONE;
      Serial.println("setup end");
      ledController->onLed();
      st = stSTANDBY;
      break;
    default:
      break;
    }
  case stSTANDBY:
    switch (ev)
    {
    case evBUTTON_PUSHED:
      ev = evNONE;
      // 何番が押されたかチェックする
      for (size_t i = 0; i < IOpinCnt; i++)
      {
        if (digitalRead(ButtonPins[i]) == LOW)
        {
          PushedButtonIdx = i;
          Serial.print("stSTANDBY evBUTTON_PUSHED ");
          Serial.print(PushedButtonIdx);
          Serial.println(" was pushed");
          break;
        }
      }

      if (PushedButtonIdx != -1)
      {
        Ranking[idxRanking] = PushedButtonIdx;
        idxRanking = (idxRanking + 1) % IOpinCnt;
        // 対応するLEDを点灯する  
        playerLedController->onLed(PushedButtonIdx);
        playerLedController->onBlinkLed(PushedButtonIdx);
        // 非同期で音声を再生する
        asyncPlaySound(SOUND_BUZZER);
        // 状態表示用LEDを点滅状態にする
        ledController->setLedBlinkCount(PushedButtonIdx);
        ledController->startBlink();
        // 解答中状態に遷移する
        PushedButtonIdx = -1;
        st = stANSWER;
      }
      break;
    default:
      break;
    }
    break;
  case stANSWER: // 解答中状態
    switch (ev)
    {
    case evBUTTON_PUSHED:
      ev = evNONE;
      // 押されたのがリセットボタンかチェックする
      if (digitalRead(CORRECT_PIN) == LOW)
      {
        PushedButtonIdx = -1;
        ev = evCORRECT_PUSHED;
        break;
      }
      if (digitalRead(WRONG_PIN) == LOW)
      {
        PushedButtonIdx = -1;
        ev = evWRONG_PUSHED;
        break;
      }

      // 何番が押されたかチェックする
      for (size_t i = 0; i < IOpinCnt; i++)
      {
        if (digitalRead(ButtonPins[i]) == LOW)
        {
          PushedButtonIdx = i;
          Serial.print("stANSWER evBUTTON_PUSHED ");
          Serial.print(PushedButtonIdx);
          Serial.println(" was pushed");
          break;
        }
      }

      // 押されていた場合（ボタンのアップ（戻り）もイベントがFireするので）
      if (PushedButtonIdx != -1)
      {
        bool isDuplicated = false;
        for (int i = 0; i < IOpinCnt; i++)
        {
          if (Ranking[i] == PushedButtonIdx)
          {
            isDuplicated = true;
            break;
          }
        }
        if (isDuplicated)
        {
          // Switchを抜ける
          break;
        }

        // 2着以降の配列に押された番号を格納
        Ranking[idxRanking] = PushedButtonIdx;
        idxRanking = (idxRanking + 1) % IOpinCnt;
        // 2着のランプを点灯
        if (Ranking[(idxCurrent + 1) % IOpinCnt] != -1)
        {
          playerLedController->onLed(Ranking[(idxCurrent + 1) % IOpinCnt]);
        }

        // // debug
        // Serial.print(Ranking[0]);
        // Serial.print(" ");
        // Serial.print(Ranking[1]);
        // Serial.print(" ");
        // Serial.print(Ranking[2]);
        // Serial.print(" ");
        // Serial.print(Ranking[3]);
        // Serial.print(" ");
        // Serial.print(Ranking[4]);
        // Serial.print(" ");
        // Serial.print(Ranking[5]);
        // Serial.print(" ");
        // Serial.print(Ranking[6]);
        // Serial.print(" ");
        // Serial.print(Ranking[7]);
        // Serial.print(" ");
        // Serial.println("");

        // 解答中状態のまま
        PushedButtonIdx = -1;
        st = stANSWER;
      }

      break;
    case evCORRECT_PUSHED:
      Serial.print("stANSWER");
      Serial.println(" evCORRECT_PUSHED");
      ev = evNONE;
      // LEDを消灯する
      // clearLeds();
      playerLedController->clearAllLeds();
      // 音を再生する
      playSound(SOUND_CORRECT);
      // v_playSound(sound_correct, sound_correct_len);
      // 点滅を止める
      ledController->stopBlink();

      // 着順関連のカウンタのリセット
      for (int i = 0; i < IOpinCnt; i++)
      {
        Ranking[i] = -1;
      }
      idxCurrent = 0;
      idxRanking = 0;
      // ボタン待機状態に戻る
      st = stSTANDBY;
      break;
    case evWRONG_PUSHED:
      Serial.print("stANSWER");
      Serial.println(" evWRONG_PUSHED");
      ev = evNONE;
      // LEDを消灯する
      // clearLeds();
      // playerLedController->clearAllLeds();
      // 音を再生する
      // v_playSound(sound_wrong, sound_wrong_len);
      playSound(SOUND_WRONG);
      // 現在の1着の点滅を止める
      playerLedController->offBlinkLed(Ranking[idxCurrent]);
      // 次の着順を点滅させる
      idxCurrent = (idxCurrent + 1) % IOpinCnt;
      if (Ranking[idxCurrent] == -1)
      {
        // playSound(SOUND_WRONG);
        // 着順関連のカウンタのリセット
        for (int i = 0; i < IOpinCnt; i++)
        {
          Ranking[i] = -1;
        }
        idxCurrent = 0;
        idxRanking = 0;
        st = stSTANDBY;
        break;
      }

      Serial.print("blink led on ");
      Serial.println(Ranking[idxCurrent]);
      playerLedController->onBlinkLed(Ranking[idxCurrent]);
      // 2着のランプを点灯
      if (Ranking[(idxCurrent + 1) % IOpinCnt] != -1)
      {
        playerLedController->onLed(Ranking[(idxCurrent + 1) % IOpinCnt]);
      }

      // ステータス表示用のLEDの再設定
      ledController->stopBlink();
      ledController->setLedBlinkCount(PushedButtonIdx);
      ledController->startBlink();

      // 解答中状態のまま
      st = stANSWER;
      // // debug
      // Serial.print(Ranking[0]);
      // Serial.print(" ");
      // Serial.print(Ranking[1]);
      // Serial.print(" ");
      // Serial.print(Ranking[2]);
      // Serial.print(" ");
      // Serial.print(Ranking[3]);
      // Serial.print(" ");
      // Serial.print(Ranking[4]);
      // Serial.print(" ");
      // Serial.print(Ranking[5]);
      // Serial.print(" ");
      // Serial.print(Ranking[6]);
      // Serial.print(" ");
      // Serial.print(Ranking[7]);
      // Serial.print(" ");
      // Serial.println("");

      break;

    default:
      break;
    }
    break;
  default:
    break;
  }
}

// PCINT2のいずれかのピンに割り込み（CHANGE)が発生したときに呼ばれる関数
ISR(PCINT0_vect)
{
  ev = evBUTTON_PUSHED;
}
// PCINT2のいずれかのピンに割り込み（CHANGE)が発生したときに呼ばれる関数
ISR(PCINT1_vect)
{
  ev = evBUTTON_PUSHED;
}
// PCINT2のいずれかのピンに割り込み（CHANGE)が発生したときに呼ばれる関数
ISR(PCINT2_vect)
{
  ev = evBUTTON_PUSHED;
}

// 音声の再生
// 初期設定
void initSoundOutput()
{
  Serial.println("initSoundOutput start");
  pinMode(SOUND_OUT_PIN, OUTPUT);
  digitalWrite(SOUND_OUT_PIN, LOW);
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  Serial.println("initSoundOutput end");
}

// // 音声を再生する
// void playSound(const unsigned char sound[], int len)
// {
//   Serial.println("play sound");
//   for (int i = 0; i < len; i++)
//   {
//     OCR2B = pgm_read_byte_near(&sound[i]);
//     delayMicroseconds(100);
//   }
//   // OCR2B = pgm_read_byte_near(0x00);
//   Serial.println("play sound end");
// }

// // すべてのLEDを消灯する
// void clearLeds()
// {
//   byte leds = 0;
//   updateShiftRegister(leds);
// }

// void LedON(int i)
// {
//   byte leds = 0;
//   leds = 1 << i;
//   // Serial.print("LED ON ");
//   // Serial.println(i);
//   updateShiftRegister(leds);
// }
// /*
//    updateShiftRegister() - This function sets the latchPin to low, then calls the Arduino function 'shiftOut' to shift out contents of variable 'leds' in the shift register before putting the 'latchPin' high again.
// */
// void updateShiftRegister(byte leds)
// {
//   digitalWrite(RCLK_74HC595, LOW);
//   shiftOut(SER_74HC595, SRCLK_74HC595, LSBFIRST, leds);
//   digitalWrite(RCLK_74HC595, HIGH);
// }

void initInterruptButtonInterrupt()
{
  Serial.println("initButtonInterrupt start");
  // 必要なピンの割り込みを有効にする
  PCICR |= B00000111;  // [X X X X X PCMSK2 PCMSK1 PCMSK0]
  PCMSK0 |= B00000011; // PCINT0_vect[X X SCK MISO MOSI D10 D9 D8] // SCKはまともに動作しなさそう
  PCMSK1 |= B00001111; // PCINT1_vect[X RESET A5 A4 A3 A2 A1 A0]
  PCMSK2 |= B11110000; // PCINT2_vect[D7 D6 D5 D4 D3 D2 D1 D0]
  Serial.println("initButtonInterrupt end");
}