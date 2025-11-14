# 📡 MQTT API Documentation - ESP32 Reservoir Monitoring System

## Overview

This document describes the MQTT API for the ESP32-based reservoir monitoring system using the JSN-SR04T ultrasonic sensor. The system provides real-time water level monitoring with configurable parameters via MQTT.

## 🔗 Connection Information

| Parameter | Description |
|-----------|-------------|
| **Protocol** | MQTT v3.1.1 |
| **Default Port** | 1883 (non-SSL) / 8883 (SSL) |
| **QoS Level** | 0 (Fire and Forget) |
| **Keep Alive** | 60 seconds |
| **Clean Session** | true |

## 📋 Topic Structure

The system uses the following topic pattern:
```
{username}/{location}/{device_type}/{device_id}/{message_type}
```

### Example Topics:
- Configuration: `adriano/floripa/reservatorio/002/settings`
- Status: `adriano/floripa/reservatorio/002/device_settings`
- Readings: `adriano/floripa/reservatorio/002/device_readings`

## 📤 Publishing to Device (Configuration)

### Topic: `{base_topic}/settings`

Send configuration commands to the device using JSON format.

#### 🔧 Available Configuration Parameters

##### **Water Level Settings**
```json
{
  "level_min": 150.0,           // Minimum water level (cm) - when tank is full
  "level_max": 25.0,            // Maximum sensor distance (cm) - when tank is empty
  "filter_threshold": 10.0      // Outlier filter threshold (%) - default: 10%
}
```

##### **Device Information**
```json
{
  "device_id": "RES_001",           // Device identifier (max 32 chars)
  "manufacturer": "AquaTech",       // Manufacturer name (max 32 chars)
  "model": "UltraSonic_v2",        // Device model (max 32 chars)
  "sensor_type": "JSN-SR04T",      // Sensor type (max 32 chars)
  "client_name": "Municipal_Water"  // Client/owner name (max 32 chars)
}
```

##### **Observation Notes**
```json
{
  "device_notes": "Reservoir A - Main Supply",     // Device info notes (max 64 chars)
  "settings_notes": "Updated threshold for stability", // Settings notes (max 64 chars)
  "readings_notes": "Normal operation"             // Readings notes (max 64 chars)
}
```

##### **Network Settings**
```json
{
  "wifi_ssid": "YourNetworkName",     // WiFi SSID (max 32 chars)
  "wifi_password": "YourPassword"     // WiFi password (max 64 chars)
}
```

#### 📝 Configuration Examples

**Basic Water Level Setup:**
```json
{
  "level_min": 180.0,
  "level_max": 20.0,
  "filter_threshold": 15.0
}
```

**Complete Device Configuration:**
```json
{
  "device_id": "TANK_NORTH_001",
  "manufacturer": "HydroSystems",
  "model": "SmartTank_Pro",
  "sensor_type": "JSN-SR04T_Waterproof",
  "client_name": "City_Water_Dept",
  "level_min": 200.0,
  "level_max": 15.0,
  "filter_threshold": 12.0,
  "device_notes": "North sector main reservoir",
  "settings_notes": "Configured for high precision",
  "readings_notes": "24/7 monitoring active"
}
```

**Filter Adjustment Only:**
```json
{
  "filter_threshold": 8.0
}
```

## 📥 Subscribing to Device Data

### 📊 Device Status - Topic: `{base_topic}/device_settings`

The device publishes its current configuration every 30 seconds.

#### Response Format:
```json
{
  "table": "device_settings",
  "device_id": "RES_001",
  "timestamp": "2025-10-20T14:30:00Z",
  "manufacturer": "AquaTech",
  "model": "UltraSonic_v2", 
  "sensor_type": "JSN-SR04T",
  "client_name": "Municipal_Water",
  "level_min_cm": 150.0,
  "level_max_cm": 25.0,
  "level_effective_cm": 125.0,
  "filter_threshold_pct": 10.0,
  "wifi_ssid": "ProductionNet",
  "wifi_rssi_dbm": -65,
  "device_notes": "Reservoir A - Main Supply",
  "settings_notes": "Standard configuration",
  "readings_notes": "Normal operation"
}
```

### 📈 Sensor Readings - Topic: `{base_topic}/device_readings`

Real-time sensor data published every 10 seconds.

#### Response Format:
```json
{
  "table": "device_readings",
  "device_id": "RES_001",
  "timestamp": "2025-10-20T14:30:15Z",
  "wifi_rssi_dbm": -67,
  "level_effective_cm": 125.0,      // Total usable tank height
  "level_available_cm": 98.5,       // Current water level (raw sensor)
  "level_available_%": 78.8,        // Water percentage (filtered)
  "temp_c": 23.4,                   // Temperature (°C)
  "humidity_%": 65.2,               // Humidity (%)
  "notes": "Normal operation"
}
```

## 🎛️ Advanced Filter Configuration

### Adaptive Threshold System

The system implements an intelligent filtering algorithm with configurable threshold:

- **Default Threshold**: 10% (configurable 0-50%)
- **Adaptive Behavior**: More permissive during initial calibration
- **Emergency Mode**: Auto-recalibration after consecutive rejections

#### Filter Behavior:
```json
{
  "filter_threshold": 10.0    // Percentage change threshold
}
```

**Threshold Guidelines:**
- `5.0-8.0`: High sensitivity (minimal filtering)
- `10.0-15.0`: Balanced (recommended for most applications) 
- `20.0-30.0`: High stability (heavy filtering)

## 🔍 Data Interpretation

### Water Level Calculations

The system measures distance from sensor to water surface and converts to percentage:

```
Water Percentage = ((level_min - measured_distance) / (level_min - level_max)) × 100
```

### Key Metrics Explanation

| Field | Description | Units | Filter Applied |
|-------|-------------|-------|----------------|
| `level_available_cm` | Actual measured water height | cm | ❌ Raw sensor data |
| `level_available_%` | Calculated water percentage | % | ✅ Adaptive filter |
| `level_effective_cm` | Total usable tank capacity | cm | ❌ Configuration value |

### Status Indicators

- **WiFi Signal**: `wifi_rssi_dbm` (dBm)
  - `-30 to -50`: Excellent
  - `-50 to -70`: Good  
  - `-70 to -90`: Weak
  - `< -90`: Poor

## 🚨 Error Handling

### Configuration Validation

The device validates all incoming configuration:

- **Numeric ranges**: Automatic clamping to valid ranges
- **String length**: Truncation at maximum lengths
- **Required fields**: Uses defaults if missing
- **Invalid JSON**: Ignored with error log

### Common Error Scenarios

**Invalid level configuration:**
```json
{
  "level_min": 25.0,     // ERROR: min < max
  "level_max": 150.0     // Will be rejected
}
```

**Threshold out of range:**
```json  
{
  "filter_threshold": 75.0   // ERROR: > 50%, clamped to 50%
}
```

## 📋 Implementation Examples

### Python MQTT Client

```python
import paho.mqtt.client as mqtt
import json
import time

# Configuration
BROKER = "your-mqtt-broker.com"
PORT = 1883
USERNAME = "your_username" 
PASSWORD = "your_password"
BASE_TOPIC = "adriano/floripa/reservatorio/002"

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    # Subscribe to device data
    client.subscribe(f"{BASE_TOPIC}/device_readings")
    client.subscribe(f"{BASE_TOPIC}/device_settings")

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        print(f"Topic: {msg.topic}")
        print(f"Data: {json.dumps(data, indent=2)}")
    except json.JSONDecodeError:
        print(f"Invalid JSON received: {msg.payload}")

# Setup client
client = mqtt.Client()
client.username_pw_set(USERNAME, PASSWORD)
client.on_connect = on_connect
client.on_message = on_message

# Connect and start loop
client.connect(BROKER, PORT, 60)

# Send configuration
config = {
    "level_min": 180.0,
    "level_max": 20.0,
    "filter_threshold": 12.0,
    "device_notes": "Updated via Python API"
}

client.publish(f"{BASE_TOPIC}/settings", json.dumps(config))

# Keep listening
client.loop_forever()
```

### Node.js MQTT Client

```javascript
const mqtt = require('mqtt');

const client = mqtt.connect('mqtt://your-mqtt-broker.com', {
    username: 'your_username',
    password: 'your_password'
});

const baseTopic = 'adriano/floripa/reservatorio/002';

client.on('connect', () => {
    console.log('Connected to MQTT broker');
    
    // Subscribe to device data
    client.subscribe(`${baseTopic}/device_readings`);
    client.subscribe(`${baseTopic}/device_settings`);
    
    // Send configuration
    const config = {
        level_min: 180.0,
        level_max: 20.0,
        filter_threshold: 15.0
    };
    
    client.publish(`${baseTopic}/settings`, JSON.stringify(config));
});

client.on('message', (topic, message) => {
    try {
        const data = JSON.parse(message.toString());
        console.log(`Topic: ${topic}`);
        console.log(`Data:`, JSON.stringify(data, null, 2));
        
        if (data.table === 'device_readings') {
            console.log(`Water Level: ${data['level_available_%']}%`);
        }
    } catch (error) {
        console.error('Invalid JSON:', message.toString());
    }
});
```

### cURL Commands

**Send Configuration:**
```bash
# Using mosquitto_pub
mosquitto_pub -h your-broker.com \
              -u username -P password \
              -t "adriano/floripa/reservatorio/002/settings" \
              -m '{"level_min": 180.0, "filter_threshold": 12.0}'

# Subscribe to readings
mosquitto_sub -h your-broker.com \
              -u username -P password \
              -t "adriano/floripa/reservatorio/002/device_readings"
```

## 🔐 Security Considerations

- Always use authentication (username/password)
- Consider using SSL/TLS for production (port 8883)
- Implement proper access control on MQTT broker
- Validate all incoming data on client side
- Monitor for unusual configuration changes

## 📞 Support & Troubleshooting

### Common Issues

1. **Device not responding**: Check network connectivity and MQTT credentials
2. **Configuration not applied**: Verify JSON format and field names
3. **Erratic readings**: Adjust `filter_threshold` value
4. **Connection drops**: Check WiFi signal strength (`wifi_rssi_dbm`)

### Debug Information

Enable verbose logging by monitoring the device serial output at 115200 baud rate for detailed operational information.

---

**Document Version**: 1.0  
**Last Updated**: October 20, 2025  
**Compatible Firmware**: ESP32_TTGO_FreeRTOS v25.10.09+