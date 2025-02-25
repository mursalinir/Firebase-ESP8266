/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt/Firebase-ESP8266
 * 
 * Copyright (c) 2022 mobizt
 *
*/

//This example shows how to store and read binary data from file on SD card to database.

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

File file;

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

#if defined(ESP8266)
      //required for large file data, increase Rx size as needed.
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 512 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";

  //To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  //Mount SD card
#if defined(ESP32)
  if (!Firebase.sdBegin(13, 14, 2, 15)) //SS, SCK,MISO, MOSI
#elif defined(ESP8266)
  if (!Firebase.sdBegin(15)) //SS
#endif
  {
    Serial.println("SD Card mounted failed");
    return;
  }

  //Delete demo files
  if (DEFAULT_SD_FS.exists("/file1.txt"))
    DEFAULT_SD_FS.remove("/file1.txt");

  if (DEFAULT_SD_FS.exists("/file2.txt"))
    DEFAULT_SD_FS.remove("/file2.txt");

  //Write demo data to file
  file = DEFAULT_SD_FS.open("/file1.txt", FILE_WRITE);
  uint8_t v = 0;
  for (int i = 0; i < 512; i++)
  {
    file.write(v);
    v++;
  }

  file.close();
}

void loop()
{

  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    //File name must be in 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension)
    Serial.printf("Set file... %s\n", Firebase.setFile(fbdo, StorageType::SD, "test/file/data", "/file1.txt") ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get file... %s\n", Firebase.getFile(fbdo, StorageType::SD, "test/file/data", "/file2.txt") ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
    {

#if defined(ESP32)
      Firebase.sdBegin(13, 14, 2, 15); //SS, SCK,MISO, MOSI
#elif defined(ESP8266)
      Firebase.sdBegin(15);  //SS
#endif

      //Readout the downloaded file
      file = DEFAULT_SD_FS.open("/file2.txt", FILE_READ);
      int i = 0;

      while (file.available())
      {
        if (i > 0 && i % 16 == 0)
          Serial.println();

        uint8_t v = file.read();

        if (v < 16)
          Serial.print("0");

        Serial.print(v, HEX);
        Serial.print(" ");
        i++;
      }

      Serial.println();
      file.close();
    }
  }
}
