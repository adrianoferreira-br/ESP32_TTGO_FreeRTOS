# Sistema de Ativação Automática de Sensores

## Visão Geral

O sistema agora possui ativação automática de sensores baseada no tipo de equipamento definido. Não é mais necessário definir manualmente cada sensor - basta definir o tipo de equipamento e os sensores apropriados serão automaticamente ativados.

## Como Usar

### 1. Definir o Tipo de Equipamento

No arquivo `src/constants.cpp`, defina apenas o tipo de equipamento:

```cpp
#define EQUIP_RESERVATORIO  // Para equipamentos tipo reservatório
// OU
#define EQUIP_PRENSA       // Para equipamentos tipo prensa
// OU
#define EQUIP_PROCESSAMENTO // Para equipamentos de processamento
// OU
#define EQUIP_LINEA        // Para equipamentos tipo linea
// OU
#define EQUIP_OUTRO        // Para outros tipos de equipamentos
```

### 2. Sensores Ativados Automaticamente

| Tipo de Equipamento | Sensores Ativados Automaticamente |
|--------------------|------------------------------------|
| **EQUIP_RESERVATORIO** | SENSOR_TEMPERATURE<br>SENSOR_WATER_LEVEL<br>SENSOR_BATIDA<br>SENSOR_BATTERY_VOLTAGE |
| **EQUIP_PRENSA**<br>**EQUIP_PROCESSAMENTO**<br>**EQUIP_LINEA** | SENSOR_BATIDA<br>SENSOR_TEMPERATURE<br>SENSOR_BATTERY_VOLTAGE |
| **EQUIP_OUTRO** | SENSOR_TEMPERATURE<br>SENSOR_BATTERY_VOLTAGE |
| **Nenhum definido** | SENSOR_BATTERY_VOLTAGE (padrão) |

## Características Especiais

### Para EQUIP_RESERVATORIO:
- **SENSOR_WATER_LEVEL**: Ativa automaticamente o sistema de ultrassom JSN-SR04T
- **Filtro Inteligente**: Sistema de filtragem percentual com:
  - Buffer circular de 10 valores
  - Detecção de outliers (threshold de 5%)
  - Análise de tendência por regressão linear
  - Correção automática baseada em tendências

### Para Equipamentos de Produção (PRENSA, PROCESSAMENTO, LINEA):
- **SENSOR_BATIDA**: Monitoramento de eventos de produção
- **SENSOR_TEMPERATURE**: Monitoramento térmico
- **SENSOR_BATTERY_VOLTAGE**: Monitoramento de energia

## Arquivo de Configuração

O sistema está implementado em `src/constants.cpp` nas linhas 8-25:

```cpp
// ============================================================================
// ATIVAÇÃO AUTOMÁTICA DOS SENSORES BASEADO NO TIPO DE EQUIPAMENTO
// ============================================================================
#if defined(EQUIP_PRENSA) || defined(EQUIP_PROCESSAMENTO) || defined(EQUIP_LINEA)
  #define SENSOR_BATIDA  
  #define SENSOR_TEMPERATURE
  #define SENSOR_BATTERY_VOLTAGE
#elif defined(EQUIP_RESERVATORIO)
  #define SENSOR_TEMPERATURE
  #define SENSOR_WATER_LEVEL 
  #define SENSOR_BATIDA
  #define SENSOR_BATTERY_VOLTAGE
#elif defined(EQUIP_OUTRO)
  #define SENSOR_TEMPERATURE
  #define SENSOR_BATTERY_VOLTAGE
#else
  // Configuração padrão quando nenhum equipamento específico é definido
  #define SENSOR_BATTERY_VOLTAGE
#endif
```

## Vantagens

1. **Simplicidade**: Define apenas o tipo de equipamento
2. **Consistência**: Garante que os sensores corretos sejam ativados
3. **Manutenibilidade**: Centralizou a lógica de ativação
4. **Flexibilidade**: Fácil de adicionar novos tipos de equipamentos
5. **Segurança**: Evita erros de configuração manual

## Exemplo Prático

### Antes (Manual):
```cpp
#define EQUIP_RESERVATORIO
#define SENSOR_WATER_LEVEL      // Tinha que definir manualmente
#define SENSOR_TEMPERATURE      // Tinha que definir manualmente
#define SENSOR_BATIDA          // Tinha que definir manualmente
#define SENSOR_BATTERY_VOLTAGE // Tinha que definir manualmente
```

### Agora (Automático):
```cpp
#define EQUIP_RESERVATORIO  // Ativa automaticamente todos os sensores necessários
```

## Integração com Sistema de Filtros

Quando `EQUIP_RESERVATORIO` é definido:
- O `SENSOR_WATER_LEVEL` é automaticamente ativado
- O sistema de filtros inteligentes do ultrassom é inicializado
- As funções `setup_ultrasonic()` e `loop_ultrasonic()` são compiladas
- O filtro percentual com detecção de outliers fica disponível

## Status de Implementação

✅ **IMPLEMENTADO E TESTADO COM SUCESSO!**

- ✅ Sistema de ativação automática funcionando
- ✅ `EQUIP_RESERVATORIO` ativa automaticamente `SENSOR_WATER_LEVEL`
- ✅ Código do ultrassom sendo inicializado corretamente
- ✅ Filtro inteligente ativo com detecção de outliers
- ✅ Logs confirmando ativação: "Ultrassônico inicializado com filtro inteligente!"

### Problema Inicial Identificado e Resolvido

**Problema:** As definições `#define` em `constants.cpp` não estavam sendo processadas corretamente devido a:
1. Dependência circular entre headers
2. Ordem incorreta de includes
3. Definições em arquivo `.cpp` em vez de `.h`

**Solução:** 
1. Movi todas as definições para `constants.h`
2. Removi dependência circular entre `main.h` e `wifi_mqtt.h`
3. Reorganizei ordem dos includes para `constants.h` ser o primeiro