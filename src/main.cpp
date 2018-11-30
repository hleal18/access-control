#include <SPI.h>
#include <MFRC522.h>
#include "LiquidCrystal_I2C.h"
#include <Servo.h>

#define RST_PIN 9                 //Pin 9 para el reset del RC522
#define SS_PIN 10                 //Pin 10 para el SS (SDA) del RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); //Creamos el objeto para el RC522

// Tutorial:
// https://naylampmechatronics.com/blog/22_Tutorial-Lector-RFID-RC522.html

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo mecanismo;
const int alarma = 7;
const int acceso_permitido = 4;
const int acceso_denegado = 5;
int pos = 0;

void activar_alarma();
void desactivar_alarma();
void permitir_acceso();
void denegar_acceso();

void setup()
{
        Serial.begin(9600); //Iniciamos la comunicación  serial
        SPI.begin();        //Iniciamos el Bus SPI
        lcd.init();
        lcd.backlight();
        mecanismo.attach(6);
        pinMode(alarma, OUTPUT);
        pinMode(acceso_permitido, OUTPUT);
        pinMode(acceso_denegado, OUTPUT);
        mfrc522.PCD_Init(); // Iniciamos  el MFRC522
        lcd.print("Lectura del UID");
        Serial.println("Lectura del UID");
}

void loop()
{
        // Revisamos si hay nuevas tarjetas  presentes
        if (mfrc522.PICC_IsNewCardPresent())
        {
                //Seleccionamos una tarjeta
                if (mfrc522.PICC_ReadCardSerial())
                {
                        lcd.setCursor(0, 0);
                        lcd.print("Nueva identificacion.");
                        lcd.setCursor(0, 1);
                        // Enviamos serialemente su UID
                        Serial.print("Card UID:");
                        for (byte i = 0; i < mfrc522.uid.size; i++)
                        {
                                Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                                Serial.print(mfrc522.uid.uidByte[i], HEX);
                                lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                                lcd.print(mfrc522.uid.uidByte[i], HEX);
                        }
                        Serial.println();
                        // Terminamos la lectura de la tarjeta  actual
                        mfrc522.PICC_HaltA();
                        activar_alarma();
                        denegar_acceso();
                        for (pos = 0; pos <= 180; pos += 1)
                        { // goes from 0 degrees to 180 degrees
                                // in steps of 1 degree
                                mecanismo.write(pos); // tell servo to go to position in variable 'pos'
                                                      // waits 15ms for the servo to reach the position
                        }
                        delay(200);
                        for (pos = 180; pos >= 0; pos -= 1)
                        {                             // goes from 180 degrees to 0 degrees
                                mecanismo.write(pos); // tell servo to go to position in variable 'pos'
                                                      // waits 15ms for the servo to reach the position
                        }
                        delay(1000);
                        desactivar_alarma();
                        permitir_acceso();
                }
        }
}

void activar_alarma()
{
        digitalWrite(alarma, HIGH);
}

void desactivar_alarma()
{
        digitalWrite(alarma, LOW);
}

void permitir_acceso()
{
        digitalWrite(acceso_permitido, HIGH);
        digitalWrite(acceso_denegado, LOW);
}

void denegar_acceso()
{
        digitalWrite(acceso_denegado, HIGH);
        digitalWrite(acceso_permitido, LOW);
}