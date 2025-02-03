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
    String Nomee;
    int sampleAvgBatida;
    float calibrationCurrent;
    float calibrationVoltage;
    int limiteSup;
    int limiteInf;
    int limiteMax;
    String ip;
    String ethernetSSID;
    String ethernetPassword;


  // put your main code here, to run repeatedly:  
    Serial.println("Lendo valor do Firebase...");
    //Nome
     if (Firebase.RTDB.getString(&firebaseData, "/B100TI/Nome")) {
        if (firebaseData.dataType() == "string") {
            Nomee = firebaseData.stringData();            
            Serial.println("Nome: " +Nomee);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //sampleAvgBatida
     if (Firebase.RTDB.getInt(&firebaseData, "/B100TI/sampleAvgBatida")) {
        if (firebaseData.dataType() == "int") {
            sampleAvgBatida = firebaseData.intData();            
            Serial.println("sampleAvgBatida: " +(String)sampleAvgBatida);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //calibrationCurrent
     if (Firebase.RTDB.getFloat(&firebaseData, "/B100TI/calibrationCurrent")) {
        if (firebaseData.dataType() == "float") {
            calibrationCurrent = firebaseData.floatData();
            Serial.println("calibrationCurrent: " +(String)calibrationCurrent);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //calibrationVoltage
     if (Firebase.RTDB.getFloat(&firebaseData, "/B100TI/calibrationVoltage")) {
        if (firebaseData.dataType() == "float") {
            calibrationVoltage = firebaseData.floatData();            
            Serial.println("calibrationVoltage: " + (String)calibrationVoltage);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //limiteSup
     if (Firebase.RTDB.getInt(&firebaseData, "/B100TI/limiteSup")) {
        if (firebaseData.dataType() == "int") {
            limiteSup = firebaseData.intData();
            Serial.println("limiteSup: " +(String)limiteSup);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //limiteInf
     if (Firebase.RTDB.getInt(&firebaseData, "/B100TI/limiteInf")) {
        if (firebaseData.dataType() == "int") {
            limiteInf = firebaseData.intData();            
            Serial.println("limiteInf: " + (String)limiteInf);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
       //limiteMax
     if (Firebase.RTDB.getInt(&firebaseData, "/B100TI/limiteMax")) {
        if (firebaseData.dataType() == "int") {
            limiteMax = firebaseData.intData();            
            Serial.println("limiteMax: " + (String)limiteMax);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //ip
     if (Firebase.RTDB.getString(&firebaseData, "/B100TI/ip")) {
        if (firebaseData.dataType() == "string") {
            ip = firebaseData.stringData();            
            Serial.println("ip: " + ip);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //ethernetSSID
     if (Firebase.RTDB.getString(&firebaseData, "/B100TI/ethernet_SSID")) {
        if (firebaseData.dataType() == "string") {
            ethernetSSID = firebaseData.stringData();            
            Serial.println("ethernetSSID: " +ethernetSSID);
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }
     //ethernetPassword
     if (Firebase.RTDB.getString(&firebaseData, "/B100TI/ethernetPassword")) {
        if (firebaseData.dataType() == "string") {
            ethernetPassword = firebaseData.stringData();            
            Serial.println("ethernetPassword: " +ethernetPassword);
            Serial.print(ethernetPassword);   
        }
     } else {        
        Serial.println("Failed to read string value. REASON: " + firebaseData.errorReason());
     }

     
  
  
   // Reatribuição da variáveis

   //String Nomee;
   AMOSTRAS_COMPROVACAO_ESTADO = sampleAvgBatida;
  
   // calibrationVoltage;
   CALIBRATION_CURRENT_FACTOR = calibrationCurrent;   
   CALIBRATION_VOLTAGE_FACTOR = calibrationVoltage;
    
    // calibrationVoltage;
    LIMIAR_SUPERIOR = (limiteSup < limiteMax)? limiteSup :  2000 - 100;
    LIMIAR_INFERIOR = (limiteInf < limiteMax)? limiteInf :  2000 - 200;
    LIMITE_MAX = (limiteMax > 1) ? limiteMax : 2000;

    //String SSID placa Ethernet;
    
    char Buffer[30];
    ethernetSSID.toCharArray(Buffer, sizeof(Buffer));
    SSID = Buffer;
    

    //ethernetPassword    
    ethernetPassword.toCharArray(Buffer, sizeof(Buffer));
    PASSWORD = Buffer;
    


/*    
    //String servMqtt;
     //int portMqtt;
     //String passwordMqtt;

// Informações para acesso ao servidor MQTT
extern char* MQTT_SERVER; //"192.168.100.4";      // Substitua pelo endereço do servidor MQTT
extern int PORT_MQTT;                             // Porta do servidor MQTT      padrão: 1883
extern char* MQTT_USERNAME;
extern char* MQTT_PASSWORD;                       // TODO: criptografar a senha em outro momento. (Cuidado com o Git)

// Análise de corrente e verificação de estado de operação do equipamento (Processando ou aguarndando insumo)

// Informações para acesso ao Firebase
extern char* FIREBASE_HOST;                       // Host do Firebase
extern char* FIREBASE_AUTH;                       // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação

    */




}


/*
 *
 */
void showValues(){



}

