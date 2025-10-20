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
    static const int BUFFER_SIZE = 5;  // Buffer pequeno para resposta rápida
    float buffer[BUFFER_SIZE];
    int currentIndex;
    int validCount;
    bool bufferFull;
    float lastValue;
    int consecutiveRejects;     // Contador de rejeições consecutivas
    int totalReadings;          // Total de leituras desde o início
    float adaptiveThreshold;    // Threshold que se adapta com o tempo
    
public:
    PercentualFilter() : currentIndex(0), validCount(0), bufferFull(false), lastValue(-1),
                        consecutiveRejects(0), totalReadings(0), adaptiveThreshold(0) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
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
    
    // FUNÇÃO PRINCIPAL: Adiciona valor e retorna resultado filtrado
    float addValue(float newValue) {
        extern float filter_threshold;  // Usa threshold configurável
        totalReadings++;
        
        // Calcular threshold adaptativo baseado no histórico
        if (totalReadings <= 10) {
            // Primeiras 10 leituras: threshold mais permissivo para calibração inicial
            adaptiveThreshold = filter_threshold * 3.0;
        } else if (totalReadings <= 30) {
            // Leituras 11-30: threshold gradualmente mais restritivo
            adaptiveThreshold = filter_threshold * (2.0 - (totalReadings - 10) / 20.0);
        } else {
            // Após 30 leituras: threshold normal
            adaptiveThreshold = filter_threshold;
        }
        
        Serial.printf("📥 LEITURA #%d: %.1f%% (threshold: %.1f%% → adaptativo: %.1f%%)\n", 
                     totalReadings, newValue, filter_threshold, adaptiveThreshold);
        
        // Verificar range básico apenas (0-100%)
        if (newValue < 0.0 || newValue > 100.0) {
            Serial.printf("❌ Fora do range 0-100%% → usando último valor\n");
            return lastValue > 0 ? lastValue : 0.0;
        }
        
        // Se é o primeiro valor válido
        if (lastValue < 0) {
            buffer[currentIndex] = newValue;
            currentIndex = (currentIndex + 1) % BUFFER_SIZE;
            validCount++;
            lastValue = newValue;
            Serial.printf("✅ PRIMEIRO VALOR: %.1f%% (será validado pelas próximas leituras)\n", newValue);
            return newValue;
        }
        
        // Calcula diferença percentual
        float diff = abs(newValue - lastValue);
        
        // Se mudança é menor que threshold adaptativo, aceita direto
        if (diff <= adaptiveThreshold) {
            buffer[currentIndex] = newValue;
            currentIndex = (currentIndex + 1) % BUFFER_SIZE;
            
            if (!bufferFull && validCount < BUFFER_SIZE) {
                validCount++;
            } else {
                bufferFull = true;
            }
            
            lastValue = newValue;
            consecutiveRejects = 0;  // Reset contador de rejeições
            
            // Suavização leve: 70% novo valor + 30% anterior
            float result = (newValue * 0.7) + (lastValue * 0.3);
            Serial.printf("✅ ACEITO (diff: %.1f%% ≤ %.1f%%) → %.1f%%\n", diff, adaptiveThreshold, result);
            return result;
        }
        
        // Para mudanças grandes, aplicar lógica adaptativa
        consecutiveRejects++;
        
        // MODO EMERGÊNCIA: Se muitas rejeições consecutivas, forçar recalibração
        if (consecutiveRejects >= 8) {
            Serial.printf("🚨 MODO EMERGÊNCIA: %d rejeições consecutivas - FORÇANDO RECALIBRAÇÃO\n", consecutiveRejects);
            
            // Reset do sistema
            for (int i = 0; i < BUFFER_SIZE; i++) {
                buffer[i] = newValue;  // Preenche buffer com novo valor
            }
            currentIndex = 0;
            validCount = BUFFER_SIZE;
            bufferFull = true;
            lastValue = newValue;
            consecutiveRejects = 0;
            totalReadings = 1;  // Reinicia contagem para nova calibração
            
            Serial.printf("🔄 SISTEMA RECALIBRADO para %.1f%%\n", newValue);
            return newValue;
        }
        
        // Para mudanças grandes, precisa de confirmação no buffer
        if (validCount < 2) {
            Serial.printf("⏳ MUDANÇA GRANDE (%.1f%%) sem confirmação → mantendo %.1f%% (rejeição #%d)\n", 
                         diff, lastValue, consecutiveRejects);
            return lastValue;
        }
        
        // Verifica se há valores similares no buffer para confirmar a tendência
        int confirmations = 0;
        int count = bufferFull ? BUFFER_SIZE : validCount;
        
        for (int i = 0; i < count; i++) {
            if (abs(buffer[i] - newValue) <= adaptiveThreshold * 1.5) {
                confirmations++;
            }
        }
        
        // Critério de confirmação mais permissivo no início
        float confirmationRatio = (totalReadings <= 20) ? 0.2 : 0.4;  // 20% inicial, 40% depois
        
        // Se confirmações suficientes, aceita mudança
        if (confirmations >= (count * confirmationRatio)) {
            buffer[currentIndex] = newValue;
            currentIndex = (currentIndex + 1) % BUFFER_SIZE;
            
            if (!bufferFull && validCount < BUFFER_SIZE) {
                validCount++;
            } else {
                bufferFull = true;
            }
            
            lastValue = newValue;
            consecutiveRejects = 0;  // Reset contador
            
            Serial.printf("✅ CONFIRMADO (%.1f%% diff, %d/%d confirmações, ratio=%.1f%%) → %.1f%%\n", 
                         diff, confirmations, count, confirmationRatio * 100, newValue);
            return newValue;
        }
        
        // Mudança não confirmada - mantém valor anterior
        Serial.printf("❌ REJEITADO (%.1f%% diff, só %d/%d confirmações, ratio=%.1f%%) → mantendo %.1f%% (rejeição #%d)\n", 
                     diff, confirmations, count, confirmationRatio * 100, lastValue, consecutiveRejects);
        return lastValue;
    }
    
    // Estatísticas detalhadas
    void printStats() {
        Serial.printf("📊 FILTRO: Última=%.1f%%, Média=%.1f%%, Leituras=%d, Rejeições=%d, Threshold=%.1f%%\n", 
                     lastValue, calculateAverage(), totalReadings, consecutiveRejects, adaptiveThreshold);
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