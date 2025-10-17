/*  https://makerhero.com/img/files/download/RCWL-1655-Datasheet.pdf
*   https://www.makerhero.com/produto/modulo-sensor-ultrassonico-impermeavel-jsn-sr04t/
*
*
*/

#include "jsn_sr04t.h"
#include "display.h"

// =====================================================
// FILTRO INTELIGENTE DE PERCENTUAL COM DETECÇÃO DE OUTLIERS
// =====================================================

class PercentualFilter {
private:
    static const int BUFFER_SIZE = 10;
    static constexpr float OUTLIER_THRESHOLD = 5.0; // 5% de tolerância
    
    float buffer[BUFFER_SIZE];
    int currentIndex;
    int validCount;
    bool bufferFull;
    
public:
    PercentualFilter() : currentIndex(0), validCount(0), bufferFull(false) {
        // Inicializar buffer com zeros
        for(int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = 0.0;
        }
    }
    
    // Calcula a média dos valores válidos no buffer
    float calculateAverage() {
        if (validCount == 0) return 0.0;
        
        float sum = 0.0;
        int count = bufferFull ? BUFFER_SIZE : validCount;
        
        for(int i = 0; i < count; i++) {
            sum += buffer[i];
        }
        return sum / count;
    }
    
    // Calcula a tendência (regressão linear simples) dos últimos valores
    float calculateTrend() {
        int count = bufferFull ? BUFFER_SIZE : validCount;
        if (count < 3) return calculateAverage(); // Mínimo 3 pontos para tendência
        
        float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
        
        for(int i = 0; i < count; i++) {
            float x = i + 1; // Posição temporal
            float y = buffer[i];
            
            sumX += x;
            sumY += y;
            sumXY += x * y;
            sumX2 += x * x;
        }
        
        // Calcular coeficiente angular (slope)
        float denominator = count * sumX2 - sumX * sumX;
        if (abs(denominator) < 0.0001) return calculateAverage(); // Evitar divisão por zero
        
        float slope = (count * sumXY - sumX * sumY) / denominator;
        float intercept = (sumY - slope * sumX) / count;
        
        // Projetar próximo valor baseado na tendência
        float nextValue = slope * (count + 1) + intercept;
        
        // Limitar entre 0 e 100
        nextValue = (nextValue < 0.0) ? 0.0 : nextValue;
        nextValue = (nextValue > 100.0) ? 100.0 : nextValue;
        
        return nextValue;
    }
    
    // Detecta se um valor é outlier comparado com a média atual
    bool isOutlier(float newValue) {
        if (validCount < 3) return false; // Primeiras leituras sempre aceitas
        
        float average = calculateAverage();
        float deviation = abs(newValue - average);
        
        return (deviation > OUTLIER_THRESHOLD);
    }
    
    // Adiciona um novo valor ao buffer com filtragem inteligente
    float addValue(float newValue) {
        float filteredValue = newValue;
        
        // Verificar se é outlier
        if (isOutlier(newValue)) {
            // Usar tendência em vez do valor outlier
            filteredValue = calculateTrend();
            
            Serial.printf("🚨 OUTLIER DETECTADO! Valor lido: %.1f%%, Média: %.1f%%, Usando tendência: %.1f%%\n", 
                         newValue, calculateAverage(), filteredValue);
        } else {
            Serial.printf("✅ Valor aceito: %.1f%% (Média atual: %.1f%%)\n", 
                         newValue, calculateAverage());
        }
        
        // Adicionar valor filtrado ao buffer
        buffer[currentIndex] = filteredValue;
        currentIndex = (currentIndex + 1) % BUFFER_SIZE;
        
        if (!bufferFull && validCount < BUFFER_SIZE) {
            validCount++;
        } else {
            bufferFull = true;
        }
        
        return filteredValue;
    }
    
    // Obtém estatísticas do filtro para debug
    void printStats() {
        Serial.printf("📊 Filtro - Média: %.1f%%, Tendência: %.1f%%, Valores: %d\n", 
                     calculateAverage(), calculateTrend(), bufferFull ? BUFFER_SIZE : validCount);
    }
};

// Instância global do filtro
PercentualFilter percentualFilter;

struct UltrasonicResult resultado;
float altura_medida = 0.0; // altura medida pelo sensor


/*  SETUP */
void setup_ultrasonic() {  
  digitalWrite(ULTRASONIC_TRIG, LOW);  
  Serial.println("Ultrassônico inicializado com filtro inteligente!");
  Serial.println("Filtro: Últimos 10 valores, detecção de outliers ±5%, correção por tendência");
}

// Função para resetar o filtro (útil para debugging ou recalibração)
void reset_percentual_filter() {
    percentualFilter = PercentualFilter();
    Serial.println("🔄 Filtro de percentual resetado!");
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
    
    // ===== APLICAR FILTRO INTELIGENTE =====
    float percentual_filtrado = percentualFilter.addValue(percentual_bruto);
    
    // Debug detalhado
    Serial.printf("📏 Distância: %.1fcm | Bruto: %.1f%% → Filtrado: %.1f%%\n", 
                 distance_cm, percentual_bruto, percentual_filtrado);
    
    // Mostrar estatísticas do filtro a cada 5 leituras
    static int debugCounter = 0;
    if (++debugCounter >= 5) {
        percentualFilter.printStats();
        debugCounter = 0;
    }

    // Usar valor filtrado como resultado final
    result.distance_cm = distance_cm;
    result.percentual = percentual_filtrado;
    result.valido = true;

    percentual_reservatorio = percentual_filtrado;

    return result;
}