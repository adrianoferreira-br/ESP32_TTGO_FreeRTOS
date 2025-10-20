/*  https://makerhero.com/img/files/download/RCWL-1655-Datasheet.pdf
*   https://www.makerhero.com/produto/modulo-sensor-ultrassonico-impermeavel-jsn-sr04t/
*
*
*/

#include "jsn_sr04t.h"
#include "display.h"

// =====================================================
// FILTRO SIMPLES PARA RESERVATÓRIO - SEM BLOQUEIOS
// =====================================================

class PercentualFilter {
private:
    static const int BUFFER_SIZE = 10;  // Buffer pequeno para resposta rápida
    float buffer[BUFFER_SIZE];
    int currentIndex;
    int validCount;
    bool bufferFull;
    float lastValue;
    
public:
    PercentualFilter() : currentIndex(0), validCount(0), bufferFull(false), lastValue(-1) {
        for(int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = 0.0;
        }
    }
    
    // Calcula média simples do buffer
    float calculateAverage() {
        if (validCount == 0) return lastValue;
        
        float sum = 0.0;
        int count = bufferFull ? BUFFER_SIZE : validCount;
        
        for(int i = 0; i < count; i++) {
            sum += buffer[i];
        }
        return sum / count;
    }
    
    // FUNÇÃO PRINCIPAL: Adiciona valor e retorna resultado suavizado
    float addValue(float newValue) {
        Serial.printf("📥 LEITURA: %.1f%%\n", newValue);
        
        // Verificar range básico apenas (0-100%)
        if (newValue < 0.0 || newValue > 100.0) {
            Serial.printf("❌ Fora do range 0-100%% → usando último valor\n");
            return lastValue > 0 ? lastValue : 0.0;
        }
        
        // SEMPRE aceitar valores válidos - sem bloqueios
        buffer[currentIndex] = newValue;
        currentIndex = (currentIndex + 1) % BUFFER_SIZE;
        
        if (!bufferFull && validCount < BUFFER_SIZE) {
            validCount++;
        } else {
            bufferFull = true;
        }
        
        lastValue = newValue;
        
        // Resultado: 60% valor atual + 40% média do buffer (suavização leve)
        float average = calculateAverage();
        float result = (newValue * 0.6) + (average * 0.4);
        
        Serial.printf("✅ RESULTADO: %.1f%% (60%% atual + 40%% média)\n", result);
        return result;
    }
    
    // Estatísticas simples
    void printStats() {
        Serial.printf("📊 FILTRO: Média=%.1f%%, Último=%.1f%%\n", 
                     calculateAverage(), lastValue);
    }
    
    // Mostra buffer
    void printBuffer() {
        Serial.print("📝 Buffer: [");
        int count = bufferFull ? BUFFER_SIZE : validCount;
        for(int i = 0; i < count; i++) {
            Serial.printf("%.1f", buffer[i]);
            if(i < count-1) Serial.print(", ");
        }
        Serial.printf("]\n");
    }
};

// Instância global do filtro
PercentualFilter percentualFilter;

struct UltrasonicResult resultado;
float altura_medida = 0.0; // altura medida pelo sensor

/*  SETUP */
void setup_ultrasonic() {  
  digitalWrite(ULTRASONIC_TRIG, LOW);  
  Serial.println("🌊 SENSOR ULTRASSÔNICO PARA RESERVATÓRIO");
  Serial.printf("📐 Configuração: Min=%.1fcm, Max=%.1fcm\n", level_max, level_min);
  Serial.println("🔧 Filtro: Aceita todas mudanças, suavização leve 60%/40%");
}

// Função para resetar o filtro (útil para debugging ou recalibração)
void reset_percentual_filter() {
    percentualFilter = PercentualFilter();
    Serial.println("🔄 Filtro resetado!");
}

/* LOOP */
void loop_ultrasonic() {
   UltrasonicResult res = ultrasonic_read();
   if (res.valido) {  
       Serial.print("Distância: ");
       Serial.print(res.distance_cm);
       Serial.print(" cm | Percentual: ");
       Serial.print(res.percentual);
       Serial.println(" %");
       show_distancia(res.distance_cm);
       show_percentual_reservatorio(res.percentual);
       altura_medida = altura_reservatorio - res.distance_cm;
   } else {
       Serial.println("Falha na leitura do ultrassônico!");
   }
}

UltrasonicResult ultrasonic_read() {
    UltrasonicResult result;
    result.distance_cm = -1;
    result.percentual = 0;
    result.valido = false;

    digitalWrite(ULTRASONIC_TRIG, LOW);
    delayMicroseconds(200);
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(15);
    digitalWrite(ULTRASONIC_TRIG, LOW);

    long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000);
    if (duration == 0) {
        Serial.println("❌ Ultrassônico: Timeout na leitura");
        return result;
    }

    float distance_cm = (duration / 2.0) * 0.0343;
    float distance_max = level_min;

    if (distance_cm > distance_max) {
        Serial.printf("❌ Ultrassônico: Distância %.1fcm acima do máximo %.1fcm\n", distance_cm, distance_max);
        return result;
    }
    
    // Calcular percentual bruto (sem filtro)
    float percentual_bruto = ((level_min - distance_cm) / (level_min - level_max)) * 100.0;
    // Limitar o percentual entre 0 e 100
    percentual_bruto = (percentual_bruto < 0) ? 0 : percentual_bruto;
    percentual_bruto = (percentual_bruto > 100) ? 100 : percentual_bruto;
    
    // ===== APLICAR FILTRO SIMPLES =====
    float percentual_filtrado = percentualFilter.addValue(percentual_bruto);
    
    // Log principal da leitura
    Serial.printf("🌊 RESERVATÓRIO: %.1fcm → %.1f%% bruto → %.1f%% filtrado\n", 
                 distance_cm, percentual_bruto, percentual_filtrado);
    
    // Mostrar estatísticas periodicamente
    static int statCounter = 0;
    if (++statCounter >= 5) {
        percentualFilter.printStats();
        percentualFilter.printBuffer();
        statCounter = 0;
    }

    // Usar valor filtrado como resultado final
    result.distance_cm = distance_cm;
    result.percentual = percentual_filtrado;
    result.valido = true;

    percentual_reservatorio = percentual_filtrado;

    return result;
}