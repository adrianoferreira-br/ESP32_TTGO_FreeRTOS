/*
 *
 */
#include <WiFi.h>
#include "extern_data.h"
#include "constants.h"
#include "wifi_mqtt.h"
#include <FirebaseESP32.h>


const char* firebaseHost = "https://presto-pr-default-rtdb.firebaseio.com/";
const char* firebaseAuth = "RSDCg4zrJcYBVcn8i9ewxRhOTzbDMYzoju0SdIuJ";


// Instancia do Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig fbConfig;

/*
 *
 */
void firebase_setup()
{
    
    fbConfig.database_url = firebaseHost;
    fbConfig.signer.tokens.legacy_token = firebaseAuth;

    Firebase.begin(&fbConfig, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("Firebase configurado!"); 

}


/*
 *
 */
void firebase_updateValues()
{
  // put your main code here, to run repeatedly:  
    Serial.println("Lendo valor do Firebase...");

     if (Firebase.RTDB.getString(&firebaseData, "/B100TI/Nome")) {
        if (firebaseData.dataType() == "string") {
            String value = firebaseData.stringData();
            Serial.print("String value: ");
            Serial.println(value);
        }
     } else {
        Serial.println("Failed to read string value");
        Serial.println("REASON: " + firebaseData.errorReason());
     }
}


/*
 *
 */
void showValues(){



}

