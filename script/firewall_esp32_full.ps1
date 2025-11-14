# ========================================
# Script para liberar comunicacao OTA com ESP32
# Libera portas para QUALQUER dispositivo na rede local
# Execute como ADMINISTRADOR
# ========================================

Write-Host "Configurando Firewall para OTA com ESP32 (todas as redes locais)..." -ForegroundColor Cyan

# Remove regras antigas
Remove-NetFirewallRule -DisplayName "ESP32 OTA - Porta 3232 (UDP) Inbound" -ErrorAction SilentlyContinue
Remove-NetFirewallRule -DisplayName "ESP32 OTA - Porta 3232 (UDP) Outbound" -ErrorAction SilentlyContinue
Remove-NetFirewallRule -DisplayName "ESP32 OTA - Portas dinamicas Inbound" -ErrorAction SilentlyContinue
Remove-NetFirewallRule -DisplayName "ESP32 OTA - Portas dinamicas Outbound" -ErrorAction SilentlyContinue
Remove-NetFirewallRule -DisplayName "Python - Permitir OTA" -ErrorAction SilentlyContinue
Remove-NetFirewallRule -DisplayName "ESP32 - Inbound Full" -ErrorAction SilentlyContinue
Remove-NetFirewallRule -DisplayName "ESP32 - Outbound Full" -ErrorAction SilentlyContinue

# Regra 1: Porta 3232 UDP INBOUND (receber resposta do ESP32)
New-NetFirewallRule -DisplayName "ESP32 OTA - Porta 3232 (UDP) Inbound" `
    -Direction Inbound `
    -Protocol UDP `
    -LocalPort 3232 `
    -Action Allow `
    -Profile Private,Domain `
    -Description "Permite comunicacao OTA na porta 3232 UDP de qualquer ESP32 na rede local"

# Regra 2: Porta 3232 UDP OUTBOUND (enviar para ESP32)
New-NetFirewallRule -DisplayName "ESP32 OTA - Porta 3232 (UDP) Outbound" `
    -Direction Outbound `
    -Protocol UDP `
    -RemotePort 3232 `
    -Action Allow `
    -Profile Private,Domain `
    -Description "Permite comunicacao OTA na porta 3232 UDP para qualquer ESP32 na rede local"

# Regra 3: Portas dinamicas TCP INBOUND (42000-43000)
New-NetFirewallRule -DisplayName "ESP32 OTA - Portas dinamicas Inbound" `
    -Direction Inbound `
    -Protocol TCP `
    -LocalPort 42000-43000 `
    -Action Allow `
    -Profile Private,Domain `
    -Description "Permite comunicacao OTA em portas dinamicas TCP (42000-43000) de qualquer ESP32"

# Regra 4: Portas dinamicas TCP OUTBOUND (42000-43000)
New-NetFirewallRule -DisplayName "ESP32 OTA - Portas dinamicas Outbound" `
    -Direction Outbound `
    -Protocol TCP `
    -RemotePort 42000-43000 `
    -Action Allow `
    -Profile Private,Domain `
    -Description "Permite comunicacao OTA em portas dinamicas TCP (42000-43000) para qualquer ESP32"

# Regra 5: Libera Python (usado pelo espota.py)
$PythonPath = "C:\Users\Adriano\.platformio\python3\python.exe"
if (Test-Path $PythonPath) {
    New-NetFirewallRule -DisplayName "Python - Permitir OTA" `
        -Direction Outbound `
        -Program $PythonPath `
        -Action Allow `
        -Profile Private,Domain `
        -Description "Permite Python executar espota.py para OTA em ESP32"
    Write-Host "- Regra para Python criada: $PythonPath" -ForegroundColor Green
}

Write-Host ""
Write-Host "Firewall configurado com sucesso!" -ForegroundColor Green
Write-Host "Regras criadas:" -ForegroundColor Yellow
Write-Host "- Porta 3232 UDP (Inbound/Outbound) - Comunicacao principal OTA" -ForegroundColor White
Write-Host "- Portas 42000-43000 TCP (Inbound/Outbound) - Portas dinamicas OTA" -ForegroundColor White
Write-Host "- Python permitido para executar OTA" -ForegroundColor White
Write-Host ""
Write-Host "Agora voce pode fazer OTA em QUALQUER ESP32 na rede local!" -ForegroundColor Cyan
Write-Host ""
Write-Host "Para verificar as regras:" -ForegroundColor Cyan
Write-Host "Get-NetFirewallRule -DisplayName '*ESP32*' | Select-Object DisplayName, Enabled" -ForegroundColor Gray
