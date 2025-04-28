# ScoutMap - Host & Port Scanner

![ScoutMap](https://i.imgur.com/0glGFjl.png)

Yo, welcome to the **ScoutMap** — a badass tool cooked up by the crew at [NetScout](https://netscout.tech). This beast runs on ESP8266 for now, scanning networks, pinging hosts, and sniffing out open ports like a cyber bloodhound. It's lean, mean, and ready to dominate your network recon game. But hold up — we're already eyeing a full-on glow-up to our custom NetScout device on ESP32 for next-level performance. Ready to dive in? Let's roll.

## What's This Thing Do?

This project is a network reconnaissance tool designed to:
- **Scan Wi-Fi networks** and connect like a boss.
- **Ping hosts** to see who's alive in the subnet or beyond.
- **Probe ports** to uncover what's open and vulnerable.
- **Upload reports** to a server for further analysis.
- **Debug like a pro** with detailed logs (in English, because we keep it real).

It's all controlled via a serial interface with commands like `scan`, `ping`, `stop`. The tool's got a progress bar for silent mode, debug mode for the nerds, and a JSON-based report system for sharing results.

**Current Platform**: ESP8266 (NodeMCU v2).  
**Future Platform**: Custom NetScout device on ESP32 (because we don't mess around).

## Features That Slap

- **Wi-Fi Scanning**: Detects all nearby networks and lets you pick one to join.
- **Host Scanning**: Pings IPs or URLs, local or remote, with ICMP for local and TCP for remote.
- **Port Scanning**: Checks single ports, ranges, or a predefined list of popular ports (20, 21, 22, 80, 443, etc.).
- **Report Upload**: Sends scan results to a server via HTTP POST in JSON format.
- **Command-Driven**: Serial interface with commands like `scan all all --silent`, `ping google.com`, or `lang en`.
- **Debug Mode**: Detailed English logs for troubleshooting like a champ.

## Getting Started

### Prerequisites
- **PlatformIO** IDE (VS Code extension recommended)
- **ESP8266 NodeMCU v2** board
- A serial terminal (e.g., PlatformIO Serial Monitor) at 115200 baud.

### Installation
1. Clone this repo
2. Open the project in VS Code with PlatformIO extension
3. PlatformIO will automatically install all required dependencies:
   - ESP8266WiFi
   - ESP8266HTTPClient
   - ArduinoJson (v6.21.4)
   - ESP8266Ping
4. Connect your ESP8266 board
5. Click "Upload" in PlatformIO
6. Open Serial Monitor and start typing commands

### Usage
Connect to a Wi-Fi network when prompted, then unleash these commands:
- `scan <hosts> <ports> [--silent|--debug]`: Scan hosts and ports (e.g., `scan all all --silent`, `scan 192.168.0.20-192.168.0.40 80,443`).
- `ping <IP|URL>`: Ping a host (e.g., `ping 8.8.8.8`).
- `stop`: Halt the current scan.
- `help`: Show the command cheat sheet.

After a scan, choose `yes` or `no` to upload the report to the server (default: `https://netscout.tech/upload`).

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

We're not here to play small. Here's the NetScout roadmap, with progress bars to show how far we've kicked ass:

- **ESP8266 Implementation** (100% 🔥)
  - Full host and port scanning functionality.
  - Wi-Fi scanning and connection.
  - Serial command interface.
  - HTTP report upload.
  - Debug mode with detailed logs.

- **ESP32 Migration** (10% 🚧)
  - Port the codebase to ESP32 for our custom NetScout device.
  - Leverage ESP32's dual-core power and extra memory.
  - Initial planning and hardware prototyping in progress.

- **Web Interface** (25% 🛠️)
  - Add a web UI for controlling scans via browser.
  - Real-time scan progress and results visualization.
  - Planned for ESP32 to handle the extra load.

- **Advanced Scanning Features** (10% 🔍)
  - Service detection on open ports (e.g., HTTP, SSH).
  - OS fingerprinting (basic research started).
  - Stealth scanning modes for low-profile ops.

- **Mobile App Integration** (0% 📱)
  - Build a mobile app to control the NetScout device.
  - Push notifications for scan completion.
  - Planned for post-ESP32 release.

- **Cloud Integration** (25% ☁️)
  - Store scan reports in a cloud dashboard.
  - Historical analysis and trend tracking.
  - API for third-party integrations.

**Overall Progress**: ~45%  
We've nailed the ESP8266 version and localization, but the ESP32 beast and fancy extras are still in the works. Stay tuned, we're moving fast.

## Contributing

Got skills? Wanna join the NetScout crew? Fork this repo, make your changes, and sling a pull request our way. We're looking for:
- Bug fixes and performance tweaks.
- Feature ideas for the ESP32 version.

Check the [Issues](https://github.com/netscout-tech/scoutmap/issues) for open tasks or drop us a line at [NetScout](https://netscout.tech).

## License

This project is licensed under the MIT License — do what you want, just give us a shoutout.

## Contact

Hit us up at [NetScout](https://netscout.tech) or join the chaos on [X](https://x.com/netscout_tech). We're always down to talk shop.

**NetScout — Scan. Analyze. Take control.**