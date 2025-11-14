# ========================================
# Script COMPLETO para configurar Firewall OTA
# Remove TODAS as regras antigas e cria novas
# Execute como ADMINISTRADOR
# ========================================

Write-Host "=== LIMPEZA COMPLETA DE REGRAS ANTIGAS ===" -ForegroundColor Yellow

# Remove TODAS as regras relacionadas a ESP32/OTA
$rulesToRemove = @(
    "ESP32*",
    "OTA*",
    "Python - Permitir OTA"
)

foreach ($rule in $rulesToRemove) {
    $found = Get-NetFirewallRule -DisplayName $rule -ErrorAction SilentlyContinue
    if ($found) {
        Write-Host "Removendo regras: $rule" -ForegroundColor Gray
        Remove-NetFirewallRule -DisplayName $rule -ErrorAction SilentlyContinue
    }
}

Write-Host ""
Write-Host "=== CRIANDO NOVAS REGRAS ===" -ForegroundColor Cyan

# REGRA 1: Porta 3232 UDP INBOUND
New-NetFirewallRule -DisplayName "ESP32 OTA - UDP 3232 In" `
    -Direction Inbound `
    -Protocol UDP `
    -LocalPort 3232 `
    -Action Allow `
    -Profile Private,Domain,Public `
    -Description "OTA ESP32: Recebe na porta 3232 UDP" | Out-Null
Write-Host "[OK] Porta 3232 UDP Inbound" -ForegroundColor Green

# REGRA 2: Porta 3232 UDP OUTBOUND
New-NetFirewallRule -DisplayName "ESP32 OTA - UDP 3232 Out" `
    -Direction Outbound `
    -Protocol UDP `
    -RemotePort 3232 `
    -Action Allow `
    -Profile Private,Domain,Public `
    -Description "OTA ESP32: Envia para porta 3232 UDP" | Out-Null
Write-Host "[OK] Porta 3232 UDP Outbound" -ForegroundColor Green

# REGRA 3: Portas dinâmicas TCP INBOUND (para o host receber)
New-NetFirewallRule -DisplayName "ESP32 OTA - TCP Dinamico In" `
    -Direction Inbound `
    -Protocol TCP `
    -LocalPort 30000-65535 `
    -Action Allow `
    -Profile Private,Domain `
    -Description "OTA ESP32: Portas dinamicas TCP para receber dados" | Out-Null
Write-Host "[OK] Portas dinamicas TCP Inbound" -ForegroundColor Green

# REGRA 4: Portas dinâmicas TCP OUTBOUND (para o host enviar)
New-NetFirewallRule -DisplayName "ESP32 OTA - TCP Dinamico Out" `
    -Direction Outbound `
    -Protocol TCP `
    -RemotePort 30000-65535 `
    -Action Allow `
    -Profile Private,Domain `
    -Description "OTA ESP32: Portas dinamicas TCP para enviar dados" | Out-Null
Write-Host "[OK] Portas dinamicas TCP Outbound" -ForegroundColor Green

# REGRA 5: Programa Python
$PythonPath = "C:\Users\Adriano\.platformio\python3\python.exe"
if (Test-Path $PythonPath) {
    New-NetFirewallRule -DisplayName "ESP32 OTA - Python" `
        -Direction Outbound `
        -Program $PythonPath `
        -Action Allow `
        -Profile Private,Domain,Public `
        -Description "OTA ESP32: Permite Python executar espota.py" | Out-Null
    Write-Host "[OK] Python permitido: $PythonPath" -ForegroundColor Green
} else {
    Write-Host "[AVISO] Python nao encontrado em: $PythonPath" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== CONFIGURACAO CONCLUIDA ===" -ForegroundColor Green
Write-Host ""
Write-Host "Regras ativas:" -ForegroundColor Cyan
Get-NetFirewallRule -DisplayName "ESP32*" | Select-Object DisplayName, Enabled, Direction | Format-Table -AutoSize
