# Configuração de Level Max/Min via MQTT

## Visão Geral

O sistema agora permite alterar os valores de `level_max` e `level_min` dinamicamente via MQTT, sem necessidade de reprogramação ou reinicialização do dispositivo.

## Como Funciona

### 1. Tópico MQTT
As configurações são enviadas através do tópico **"settings"** do dispositivo.

**Exemplo de tópico completo:**
```
Adriano/florianopolis/reservatorio/002/settings
```

# Configuração Completa via MQTT

## Visão Geral

O sistema agora permite alterar **todas as configurações principais** dinamicamente via MQTT, incluindo:
- **Níveis do Reservatório:** `level_max`, `level_min`, `sample_time_s`
- **Configurações WiFi:** `wifi_ssid`, `wifi_password`
- **Configurações MQTT:** `mqtt_server`, `mqtt_port`, `mqtt_user`, `mqtt_password`

Tudo através de um único tópico **"settings"** unificado, sem necessidade de reprogramação ou reinicialização manual.

## Como Funciona

### 1. Tópico MQTT
Todas as configurações são enviadas através do tópico **"settings"** do dispositivo.

**Exemplo de tópico completo:**
```
Adriano/florianopolis/reservatorio/002/settings
```

### 2. Formato da Mensagem JSON Completa

```json
{
  "level_max": 25.5,
  "level_min": 95.0,
  "sample_time_s": 30,
  "wifi_ssid": "NovaRede_WiFi",
  "wifi_password": "novaSenha123",
  "mqtt_server": "novo.servidor.com",
  "mqtt_port": 1883,
  "mqtt_user": "novoUsuario",
  "mqtt_password": "novaSenhaMQTT"
}
```

**Todos os campos são opcionais!** Você pode enviar apenas os campos que deseja alterar.

### 3. Campos Suportados

#### 🏠 Configurações do Reservatório
- `level_max`: Altura máxima do reservatório em cm (nível 100%)
- `level_min`: Altura mínima do reservatório em cm (nível 0%)  
- `sample_time_s`: Intervalo de amostragem em segundos

#### 📶 Configurações WiFi
- `wifi_ssid`: Nome da rede WiFi
- `wifi_password`: Senha da rede WiFi

#### 📡 Configurações MQTT
- `mqtt_server`: Endereço do servidor MQTT
- `mqtt_port`: Porta do servidor MQTT (padrão: 1883)
- `mqtt_user`: Usuário para autenticação MQTT
- `mqtt_password`: Senha para autenticação MQTT

### 4. Processo de Atualização

1. **Recepção:** Sistema recebe a mensagem JSON via MQTT
2. **Validação:** Verifica se o JSON é válido
3. **Salvamento:** Grava os novos valores na flash (NVS)
4. **Atualização:** Aplica imediatamente as configurações possíveis
5. **Reset de Filtro:** Reseta o filtro percentual (se `level_max`/`level_min` mudaram)
6. **Confirmação:** Envia mensagem detalhada de confirmação via MQTT
7. **Avisos:** Informa sobre necessidade de reconexão (WiFi/MQTT)

### 5. Confirmação Automática Expandida

Após receber e processar as configurações, o sistema envia automaticamente uma confirmação completa:

```json
{
  "table": "settings_confirmation",
  "device_id": "adriano-fln-l01-tst-001",
  "timestamp": 1729123456,
  "level_max_cm": 25.5,
  "level_min_cm": 95.0,
  "level_effective_cm": 69.5,
  "sample_time_s": 30,
  "wifi_ssid": "NovaRede_WiFi",
  "wifi_status": "connected",
  "wifi_rssi": -65,
  "wifi_ip": "192.168.1.100",
  "mqtt_server": "novo.servidor.com",
  "mqtt_port": 1883,
  "mqtt_user": "novoUsuario",
  "mqtt_status": "connected",
  "status": "settings_updated",
  "message": "Configurações atualizadas com sucesso"
}
```

## Comandos de Reconexão

Para aplicar imediatamente as novas configurações de conectividade:

### 🔄 Forçar Reconexão WiFi
**Tópico:** `reconnect_wifi`
**Mensagem:** (qualquer conteúdo)
```bash
mosquitto_pub -h srv.vamodale.com -p 1883 \
  -u indx4 -P indx4_senha \
  -t "Adriano/florianopolis/reservatorio/002/reconnect_wifi" \
  -m "reconnect"
```

### 🔄 Forçar Reconexão MQTT
**Tópico:** `reconnect_mqtt`
**Mensagem:** (qualquer conteúdo)
```bash
mosquitto_pub -h srv.vamodale.com -p 1883 \
  -u indx4 -P indx4_senha \
  -t "Adriano/florianopolis/reservatorio/002/reconnect_mqtt" \
  -m "reconnect"
```

## Exemplos de Uso

### 1. Alterar apenas Configurações do Reservatório
```json
{
  "level_max": 20.0,
  "level_min": 80.0
}
```

### 2. Alterar apenas WiFi
```json
{
  "wifi_ssid": "MinhaNovaRede",
  "wifi_password": "minhaNovaSenha"
}
```

### 3. Alterar apenas MQTT
```json
{
  "mqtt_server": "meu.servidor.com",
  "mqtt_port": 8883,
  "mqtt_user": "meuUsuario"
}
```

### 4. Configuração Completa de uma só vez
```json
{
  "level_max": 22.5,
  "level_min": 88.0,
  "sample_time_s": 60,
  "wifi_ssid": "EmpresaWiFi",
  "wifi_password": "senhaSegura123",
  "mqtt_server": "iot.empresa.com",
  "mqtt_port": 1883,
  "mqtt_user": "sensor001",
  "mqtt_password": "mqttPass456"
}
```

### 5. Comando Completo via Terminal

```bash
mosquitto_pub -h srv.vamodale.com -p 1883 \
  -u indx4 -P indx4_senha \
  -t "Adriano/florianopolis/reservatorio/002/settings" \
  -m '{
    "level_max": 25.0,
    "level_min": 90.0,
    "mqtt_server": "novo.servidor.com",
    "wifi_ssid": "NovaRede"
  }'
```

## Logs de Debug Expandidos

O sistema fornece logs detalhados durante todo o processo:

```
Mensagem recebida. Tópico: settings. : {"level_max": 25.0, "wifi_ssid": "NovaRede", "mqtt_server": "novo.srv.com"}

✅ Salvo level_max: 25.0 cm
✅ Salvo WiFi SSID: NovaRede
✅ Salvo MQTT Server: novo.srv.com
🔄 Filtro percentual resetado para aplicar novos limites

📤 Enviando confirmação de configurações via MQTT...
✅ Confirmação de configurações enviada com sucesso!

📋 Configurações atuais:
   🏠 RESERVATÓRIO:
      • Level Max: 25.0 cm
      • Level Min: 90.0 cm
      • Altura Útil: 65.0 cm
      • Intervalo: 30 segundos
   📶 WIFI:
      • SSID: NovaRede
      • Status: Conectado
      • IP: 192.168.1.100
   📡 MQTT:
      • Servidor: novo.srv.com
      • Porta: 1883
      • Usuário: indx4
      • Status: Conectado

⚠️  WiFi configurações alteradas - Reconexão necessária
   Use o comando 'reconnect_wifi' ou reinicie o dispositivo
⚠️  MQTT configurações alteradas - Reconexão necessária
   Use o comando 'reconnect_mqtt' ou reinicie o dispositivo
```

### 3. Processo de Atualização

1. **Recepção:** Sistema recebe a mensagem JSON via MQTT
2. **Validação:** Verifica se o JSON é válido
3. **Salvamento:** Grava os novos valores na flash (NVS)
4. **Atualização:** Aplica imediatamente as novas configurações
5. **Reset de Filtro:** Reseta o filtro percentual para aplicar novos limites
6. **Confirmação:** Envia mensagem de confirmação via MQTT

### 4. Confirmação Automática

Após receber e processar as configurações, o sistema envia automaticamente uma confirmação:

```json
{
  "table": "settings_confirmation",
  "device_id": "adriano-fln-l01-tst-001",
  "timestamp": 1729123456,
  "level_max_cm": 25.5,
  "level_min_cm": 95.0,
  "level_effective_cm": 69.5,
  "sample_time_s": 30,
  "status": "settings_updated",
  "message": "Configurações atualizadas com sucesso"
}
```

## Exemplos de Uso

### 1. Alterar apenas Level Max
```json
{
  "level_max": 20.0
}
```

### 2. Alterar apenas Level Min
```json
{
  "level_min": 80.0
}
```

### 3. Alterar múltiplas configurações
```json
{
  "level_max": 22.5,
  "level_min": 88.0,
  "sample_time_s": 60
}
```

### 4. Usando MQTT Explorer ou Linha de Comando

**Mosquitto Publish:**
```bash
mosquitto_pub -h srv.vamodale.com -p 1883 \
  -u indx4 -P indx4_senha \
  -t "Adriano/florianopolis/reservatorio/002/settings" \
  -m '{"level_max": 25.0, "level_min": 90.0}'
```

## Logs de Debug

O sistema fornece logs detalhados durante o processo:

```
Mensagem recebida. Tópico: settings. : {"level_max": 25.0, "level_min": 90.0}
✅ Salvo level_max: 25.0 cm
✅ Salvo level_min: 90.0 cm
🔄 Filtro percentual resetado para aplicar novos limites
📤 Enviando confirmação de configurações via MQTT...
✅ Confirmação de configurações enviada com sucesso!
📋 Configurações atuais:
   • Level Max: 25.0 cm
   • Level Min: 90.0 cm
   • Altura Útil: 65.0 cm
   • Intervalo: 30 segundos
```

## Validações e Segurança

### Validações Automáticas
- **JSON Válido:** Sistema valida sintaxe JSON antes de processar
- **Campos Opcionais:** Apenas os campos enviados são atualizados
- **Persistência:** Valores são salvos na flash para sobreviver a reinicializações
- **Segurança:** Senhas não são exibidas nos logs (mostradas como [HIDDEN])

### Limites e Recomendações
- **level_max:** Entre 20 e 400 cm (validação no `state.cpp`)
- **level_min:** Deve ser maior que `level_max`
- **sample_time_s:** Recomendado entre 10 e 3600 segundos
- **wifi_ssid:** Máximo 32 caracteres
- **wifi_password:** Recomendado WPA2/WPA3
- **mqtt_server:** IP ou hostname válido
- **mqtt_port:** Portas comuns: 1883 (não-SSL), 8883 (SSL)

### Segurança de Senhas
- Senhas WiFi e MQTT são salvas criptografadas na NVS
- Não aparecem em logs ou confirmações MQTT
- Marcadas como `[HIDDEN]` nos logs de debug

## Integração com Sistema Existente

### Compatibilidade Total
- ✅ **Filtro Inteligente:** Reset automático para aplicar novos limites
- ✅ **Cálculos Percentuais:** Atualização imediata das fórmulas
- ✅ **Display:** Mostra novos valores automaticamente
- ✅ **Web Server:** Interface web reflete mudanças instantaneamente
- ✅ **Persistência:** Todas as configurações mantidas após reboot
- ✅ **Conectividade:** Reconexão automática ou manual via comandos

### Variáveis Globais Afetadas
- `level_max`, `level_min`, `SAMPLE_INTERVAL` (níveis)
- `MQTT_SERVER`, `PORT_MQTT`, `MQTT_USERNAME`, `MQTT_PASSWORD` (MQTT)
- Configurações WiFi carregadas via NVS no próximo boot
- `altura_reservatorio` (indiretamente)
- Filtro percentual (resetado quando necessário)

### Fluxo de Reconexão
1. **Configurações Salvas:** NVS atualizada imediatamente
2. **Aplicação Imediata:** Variáveis globais atualizadas
3. **Aviso ao Usuário:** Logs informam sobre necessidade de reconexão
4. **Comando Manual:** `reconnect_wifi` ou `reconnect_mqtt`
5. **Ou Automático:** Próximo reboot carrega novas configurações

## Comandos Disponíveis

### 📋 Configurações (settings)
**Formato:** JSON com campos opcionais
**Exemplo:** `{"level_max": 25, "mqtt_server": "novo.com"}`

### 🔄 Reconexões
- **reconnect_wifi:** Reconecta WiFi com novas configurações
- **reconnect_mqtt:** Reconecta MQTT com novas configurações  
- **Reboot_:** Reinicia completamente o dispositivo

### 📞 Informações
- **info:** Solicita envio de informações do dispositivo
- **settings:** Também pode ser usado para consultar configurações atuais

## Solução de Problemas

### Erro de JSON
```
❌ Erro ao analisar a mensagem de configurações JSON.
```
**Soluções:**
- Verificar sintaxe JSON (aspas duplas, vírgulas, chaves)
- Usar ferramentas online para validar JSON
- Conferir caracteres especiais

### Falha no Envio de Confirmação
```
❌ Falha ao enviar confirmação de configurações!
```
**Soluções:**
- Verificar conexão MQTT estável
- Confirmar tópico e permissões de publicação
- Verificar tamanho da mensagem (limite do broker)

### Configurações Não Aplicadas
**WiFi/MQTT:**
- Usar comandos `reconnect_wifi` ou `reconnect_mqtt`
- Ou reiniciar dispositivo com `Reboot_`

**Reservatório:**
- Verificar se valores estão dentro dos limites
- Confirmar que variáveis foram atualizadas nos logs

### Problemas de Conectividade
```
⚠️ WiFi/MQTT configurações alteradas - Reconexão necessária
```
**Soluções:**
1. **Imediata:** Usar comandos de reconexão
2. **Segura:** Reiniciar dispositivo
3. **Verificação:** Aguardar logs de confirmação

## Implementação Técnica

### Arquivos Modificados
- `src/wifi_mqtt.cpp`: Implementação principal expandida
- `include/wifi_mqtt.h`: Declarações das funções  
- `include/constants.h`: Definições das chaves NVS
- Integração com `src/state.cpp` (variáveis globais)
- Integração com `src/jsn_sr04t.cpp` (reset do filtro)

### Fluxo de Execução Completo
1. **callback()** → recebe mensagem MQTT no tópico "settings"
2. **deserializeJson()** → valida JSON com todos os campos
3. **prefs.begin("settings")** → abre namespace NVS
4. **Processamento por categoria:**
   - Reservatório: `level_max`, `level_min`, `sample_time_s`
   - WiFi: `wifi_ssid`, `wifi_password`  
   - MQTT: `mqtt_server`, `mqtt_port`, `mqtt_user`, `mqtt_password`
5. **prefs.putFloat/putString/putInt()** → salva na flash
6. **Atualização imediata** → aplica valores nas variáveis globais
7. **reset_percentual_filter()** → reseta filtro se necessário
8. **mqtt_send_settings_confirmation()** → confirma alterações
9. **Avisos de reconexão** → informa sobre mudanças de conectividade

### Segurança e Robustez
- **Validação JSON** antes de qualquer processamento
- **Campos opcionais** - apenas os enviados são alterados
- **Persistência garantida** - NVS confiável  
- **Logs detalhados** - rastreabilidade completa
- **Senhas protegidas** - não expostas em logs
- **Reconexão controlada** - evita loops de reconexão