#include <SPI.h>
#include <MFRC522.h>
#include "LiquidCrystal_I2C.h"
#include <Servo.h>

#define RST_PIN 9                 //Pin 9 para el reset del RC522
#define SS_PIN 10                 //Pin 10 para el SS (SDA) del RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); //Creamos el objeto para el RC522

const int USUARIOS = 3;
const int UID_SIZE = 4;

const String nombre_usuarios[USUARIOS] = {
    "Lucas",
    "Daniel",
    "Nicolas"};

const byte uid_usuarios[USUARIOS][UID_SIZE] = {
    {0x56, 0xFC, 0xD5, 0x1F},
    {0xE2, 0xF8, 0x32, 0x53},
    {0x40, 0x7D, 0X0E, 0X7C}};
// Tutorial:
// https://naylampmechatronics.com/blog/22_Tutorial-Lector-RFID-RC522.html

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo mecanismo;

const int alarma = 7;
const int acceso_permitido = 5;
const int acceso_denegado = 4;
const int tiempo_limite = 60000;
int tiempo = 0;
int pos = 0;

void imprimir(const char *primera_linea, const char *segunda_linea = nullptr, bool clear1 = false, bool clear2 = false);
void imprimir_introduccion();
void limpiar_pantalla();

//Tiene que ser de tamaño cuatro
int comprobar_usuario(byte uid[]);

void permitir_acceso(int index);
void denegar_acceso();

void activar_alarma();
void desactivar_alarma();

void abrir_puerta();
void cerrar_puerta();

void reiniciar_sistema();

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
        imprimir_introduccion();
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
                        imprimir("Tag detectado", "", true, true);
                        byte uid[4] = {0};
                        Serial.print("Card UID:");
                        for (byte i = 0; i < mfrc522.uid.size && i < UID_SIZE; i++)
                        {
                                Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                                Serial.print(mfrc522.uid.uidByte[i], HEX);
                                lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                                lcd.print(mfrc522.uid.uidByte[i], HEX);
                                uid[i] = mfrc522.uid.uidByte[i];
                        }
                        Serial.println();
                        Serial.println("UID detectado.");
                        for (int i = 0; i < UID_SIZE; i++)
                        {
                                Serial.print(uid[i]);
                                Serial.print(" ");
                        }

                        int usuario_index = comprobar_usuario(uid);
                        if (usuario_index != -1)
                        {
                                permitir_acceso(usuario_index);
                        }
                        else
                        {
                                denegar_acceso();
                        }
                        // Terminamos la lectura de la tarjeta  actual
                        mfrc522.PICC_HaltA();
                        delay(1000);
                }
        }
}

int comprobar_usuario(byte uid[])
{
        for (int i = 0; i < USUARIOS; i++)
        {
                for (int j = 0; j < UID_SIZE; j++)
                {
                        if (uid_usuarios[i][j] != uid[j])
                        {
                                break;
                        }
                        else if (j == UID_SIZE - 1)
                        {
                                return i;
                        }
                }
        }

        return -1;
}

void permitir_acceso(int index)
{
        digitalWrite(acceso_permitido, HIGH);
        digitalWrite(acceso_denegado, LOW);
        imprimir("Bienvenido", nombre_usuarios[index].c_str(), true, true);
        abrir_puerta();
        reiniciar_sistema();
}

void denegar_acceso()
{
        digitalWrite(acceso_denegado, HIGH);
        digitalWrite(acceso_permitido, LOW);
        imprimir("Usuario", "No encontrado.", true, true);
        activar_alarma();
        delay(2000);
        reiniciar_sistema();
}

void reiniciar_sistema()
{
        digitalWrite(acceso_denegado, HIGH);
        digitalWrite(acceso_permitido, LOW);
        imprimir("Puerta cerrada.", "Pase tarjeta.", true, true);
        desactivar_alarma();
        cerrar_puerta();
}

void activar_alarma()
{
        digitalWrite(alarma, HIGH);
}

void desactivar_alarma()
{
        digitalWrite(alarma, LOW);
}

void abrir_puerta()
{
        if (pos == 180)
                return;
        for (pos = 0; pos < 180; pos++)
        {
                mecanismo.write(pos);
                delay(5);
        }
        delay(10000);
}

void cerrar_puerta()
{
        if (pos == 0)
                return;
        for (pos = 180; pos > 0; pos--)
        {
                mecanismo.write(pos);
                delay(5);
        }
}

void imprimir_introduccion()
{
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Puerta cerrada");
        lcd.setCursor(0, 1);
        lcd.print("Pase tarjeta.");
}

void limpiar_pantalla()
{
        lcd.clear();
}

void imprimir(const char *primera_linea, const char *segunda_linea = nullptr, bool clear1 = false, bool clear2 = false)
{
        if (primera_linea)
        {
                if (clear1)
                {
                        lcd.setCursor(0, 0);
                        for (int i = 0; i < 16; i++)
                        {
                                lcd.print(" ");
                        }
                }
                lcd.setCursor(0, 0);
                lcd.print(primera_linea);
        }
        if (segunda_linea)
        {
                if (clear2)
                {
                        lcd.setCursor(0, 1);
                        for (int i = 0; i < 16; i++)
                        {
                                lcd.print(" ");
                        }
                }
                lcd.setCursor(0, 1);
                lcd.print(segunda_linea);
        }
}