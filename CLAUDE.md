# Mihashi Dev Project - Claude Instructions

## Project Overview

**Mihashi Dev Project** is an autonomous development project where GhostPC (Ubuntu 22.04) uses Claude Code to automatically develop the "Mihashi" USB MIDI Host device during nighttime hours (24:00-05:00). The system consists of three devices working together for development and monitoring.

## System Architecture

- **Mihashi** (RP2350A USB PIO HOST): Target device being developed autonomously
- **LittleJoe** (XIAO SAMD21): TinyUSB MIDI Monitor → UART ASCII output  
- **UartMonitor** (XIAO SAMD21): UART → USB Serial bridge for GhostPC monitoring
- **GhostPC** (Ubuntu 22.04): Autonomous development execution environment

## Development Context

### Current Status (2025-06-24)
- ✅ GhostPC autonomous development environment setup
- ✅ Project architecture and documentation complete
- ✅ GitHub automation and systemd integration ready
- ✅ Device specifications finalized with XIAO SAMD21 platform
- 🚧 Next: XIAO SAMD21 firmware implementation for monitoring system

### Key Technical Decisions
- Simplified from 4-device to 3-device architecture (removed picoprobe)
- Standardized on XIAO SAMD21 for both monitoring devices
- TinyUSB MIDI library for native USB MIDI support
- Arduino IDE development environment for consistency
- **IMPORTANT**: Mihashi PIO-USB Host uses GPIO 12 (D-) and GPIO 13 (D+) - NOT GPIO 0,1

### Autonomous Development Goals
- GhostPC automatically improves Mihashi firmware using Claude Code
- Real-time MIDI monitoring through LittleJoe → UartMonitor chain
- Discord notifications for development progress
- Continuous integration with GitHub Actions

## Personal Interactions and Development Notes

### User (kurogedelic) Characteristics
- **Highly systematic**: Approaches complex projects with clear architectural thinking
- **Detail-oriented**: Requests specific technical specifications and comprehensive documentation  
- **Iterative refinement**: Continuously improves and clarifies system design through discussion
- **Practical focus**: Prioritizes working implementations over theoretical perfection
- **Communication style**: Direct, technical, expects concise responses

### Notable Interaction Patterns
- Prefers structured planning before implementation
- Values autonomous systems that can operate independently
- Appreciates thorough documentation with technical depth
- Quick to adapt and refine designs based on practical considerations
- Expects commit messages and documentation to be professional and detailed

### Project Evolution Insights
- Started as "MidiBridge Project" → evolved to "Mihashi Dev Project" 
- Initial complexity was systematically reduced for practical implementation
- User demonstrated strong understanding of embedded systems and autonomous development concepts
- Showed preference for proven platforms (XIAO SAMD21) over cutting-edge options

### Development Approach Preferences
- Comprehensive README with system diagrams (Mermaid)
- Separate technical specifications for each component
- Clear file organization and project structure
- Regular git commits with detailed commit messages
- Documentation-driven development workflow

## Instructions for Future Claude Interactions

1. **Project Focus**: Always consider this as an autonomous development project where GhostPC develops Mihashi
2. **Documentation**: Maintain comprehensive technical documentation for all components
3. **System Understanding**: Remember the 3-device monitoring chain architecture
4. **Development Approach**: Prioritize working implementations with Arduino IDE ecosystem
5. **Communication**: Keep responses concise and technically accurate
6. **Progress Tracking**: Use todo lists and structured project management
7. **Always Commit**: End every work session by committing all changes with detailed commit messages
8. **IMPORTANT - Default Work Environment**: Unless explicitly specified otherwise, ALWAYS work on GhostPC (Ubuntu 22.04). All devices (Mihashi, LittleJoe, UartMonitor) are connected to GhostPC, not Mac. Use SSH to connect to ghost@192.168.11.89 with password 'deargod1986'.

## Autonomous Firmware Upload Capability

### Automatic Bootloader Trigger
- **RP2350**: Open USB Serial port at 3000bps briefly to trigger UF2 bootloader mode
- **SAMD21**: Open USB Serial port at 1200bps briefly to trigger UF2 bootloader mode
- Use `/scripts/upload_firmware.py` for automated firmware deployment

### Upload Commands
```bash
# Upload Mihashi firmware (RP2350)
python3 scripts/upload_firmware.py mihashi

# Upload LittleJoe firmware (XIAO SAMD21)  
python3 scripts/upload_firmware.py littlejoe

# Upload UartMonitor firmware (XIAO SAMD21)
python3 scripts/upload_firmware.py uartmonitor
```

## Requested Work Journal Maintenance

- User explicitly requested diary maintenance with direct, honest documentation
- Work journal should include technical progress and personal observations
- Located in `/diary/` subdirectory within project
- Document both technical achievements and interaction insights