# ScoutMap - Host & Port Scanner

![ScoutMap](https://i.imgur.com/0glGFjl.png)

Yo, welcome to the **ScoutMap** — a badass tool cooked up by the crew at [NeoScout](https://neoscout.ru). This beast runs on ESP32-C3, scanning networks, pinging hosts, and sniffing out open ports like a cyber bloodhound. It's lean, mean, and ready to dominate your network recon game.

## What's This Thing Do?

This project is a network reconnaissance tool designed to:
- **Scan Wi-Fi networks** and connect like a boss.
- **Ping hosts** to see who's alive in the subnet or beyond.
- **Probe ports** to uncover what's open and vulnerable.
- **Upload reports** to a server for further analysis.
- **Debug like a pro** with detailed logs (in English, because we keep it real).
- **Secure credential storage** with encrypted storage.
- **API integration** for cloud services and automation.

It's all controlled via a serial interface with commands like `scan`, `ping`, `stop`. The tool's got a progress bar for silent mode, debug mode for the nerds, and a JSON-based report system for sharing results.

**Current Platform**: ESP32-C3 (DevKitM-1).

## Features That Slap

- **Wi-Fi Scanning**: Detects all nearby networks and lets you pick one to join.
- **Host Scanning**: Pings IPs or URLs, local or remote, with ICMP for local and TCP for remote.
- **Port Scanning**: Checks single ports, ranges, or a predefined list of popular ports (20, 21, 22, 80, 443, etc.).
- **Report Upload**: Sends scan results to a server via HTTP POST in JSON format.
- **Command-Driven**: Serial interface with commands like `scan all all --silent`, `ping google.com`, or `lang en`.
- **Debug Mode**: Detailed English logs for troubleshooting like a champ.
- **Secure Storage**: Safely stores Wi-Fi credentials and API keys.
- **Web Interface**: JSON-based API for browser control and automation.
- **Cloud Integration**: Upload scan results and manage devices remotely.

## Getting Started

### Prerequisites
- **PlatformIO** IDE (VS Code extension recommended)
- **ESP32-C3 DevKitM-1** board
- A serial terminal (e.g., PlatformIO Serial Monitor) at 115200 baud.

### Installation
1. Clone this repo
2. Open the project in VS Code with PlatformIO extension
3. PlatformIO will automatically install all required dependencies:
   - WiFi
   - HTTPClient
   - ArduinoJson (v6.21.3)
   - ESP32Ping
   - Preferences
4. Connect your ESP32-C3 board
5. Click "Upload" in PlatformIO
6. Open Serial Monitor and start typing commands

### Usage
Connect to a Wi-Fi network when prompted, then unleash these commands:
- `scan <hosts> <ports> [--silent|--debug]`: Scan hosts and ports (e.g., `scan all all --silent`, `scan 192.168.0.20-192.168.0.40 80,443`).
- `ping <IP|URL>`: Ping a host (e.g., `ping 8.8.8.8`).
- `stop`: Halt the current scan.
- `help`: Show the command cheat sheet.
- `clear`: Clear saved Wi-Fi credentials.

After a scan, choose `yes` or `no` to upload the report to the server (default: `https://neoscout.ru/upload`).

## Project Structure
```
scoutmap/
├── src/                # Source files
│   ├── main.cpp       # Entry point
│   ├── wifi_utils.cpp # Wi-Fi functionality
│   ├── scan_utils.cpp # Scanning logic
│   ├── http_utils.cpp # HTTP communication
│   ├── command_utils.cpp # Command processing
│   ├── structs.cpp    # Data structures
│   └── config.cpp     # Configuration
├── include/           # Header files
├── lib/              # External libraries
├── test/             # Test files
└── platformio.ini    # PlatformIO configuration
```

## Roadmap

We're not here to play small. Here's the NeoScout roadmap, with progress bars to show how far we've kicked ass:

- **ESP32-C3 Implementation** (100% 🔥)
  - Full host and port scanning functionality.
  - Wi-Fi scanning and connection.
  - Serial command interface.
  - HTTP report upload.
  - Debug mode with detailed logs.
  - Secure credential storage.
  - Web API support.

- **Web Interface** (75% 🛠️)
  - Add a web UI for controlling scans via browser.
  - Real-time scan progress and results visualization.
  - JSON API for automation.

- **Advanced Scanning Features** (25% 🔍)
  - Service detection on open ports (e.g., HTTP, SSH).
  - OS fingerprinting (basic research started).
  - Stealth scanning modes for low-profile ops.

- **Mobile App Integration** (10% 📱)
  - Build a mobile app to control the NeoScout device.
  - Push notifications for scan completion.

- **Cloud Integration** (50% ☁️)
  - Store scan reports in a cloud dashboard.
  - Historical analysis and trend tracking.
  - API for third-party integrations.
  - Device management and monitoring.

**Overall Progress**: ~65%  
We've nailed the ESP32-C3 version with secure storage and web API, but the fancy extras are still in the works. Stay tuned, we're moving fast.

## Contributing

Got skills? Wanna join the NeoScout crew? Fork this repo, make your changes, and sling a pull request our way. We're looking for:
- Bug fixes and performance tweaks.
- Feature ideas for the ESP32-C3 version.
- Web interface improvements.
- Cloud integration features.

Check the [Issues](https://github.com/neoscout.ru/scoutmap/issues) for open tasks or drop us a line at [NeoScout](https://neoscout.ru).

## License

This project is licensed under the MIT License — do what you want, just give us a shoutout.

## Contact

Hit us up at [NeoScout](https://neoscout.ru) or join the chaos on [X](https://x.com/neoscout.ru). We're always down to talk shop.

**NeoScout — Scan. Analyze. Take control.**