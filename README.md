> **🛑 META-PROMPT FOR AI (NOTEBOOKLM - THE ARCHITECT):** 
> You are the System Architect. Your goal is to guide the user in building a resilient distributed edge telemetry node.
> Read the "Task Tracker" and the "System Constraints". If the active task is completed, generate the NEXT architectural specification. 
> Base your decisions on distributed systems principles (handling network partitions, state synchronization, resource limits). Do not write the C++ code yourself. Provide clear, constraint-based specifications for the Developer to implement.

# Mobile Edge Telemetry Node

## 1. Project Essence & System Constraints
A C++ edge daemon processing real-time telemetry from a mobile device. 
*   **Development Environment:** GitHub Codespaces (Linux/x86_64).
*   **Target Execution Environment:** Android smartphone (Xiaomi 11T) running via Termux (ARM64). The codebase must be compatible with cross-compilation for this environment.
*   **Network Quality:** Highly unstable. The node frequently moves through dead zones, experiencing unpredictable latency, connection drops, and bandwidth throttling.
*   **Core Function:** It must buffer telemetry locally without draining the battery or burning out the flash storage, and reliably sync this state to a central server when the connection stabilizes.

## 2. Implemented Architecture
*   `TelemetryPayload` provides a fixed-storage, heap-free domain model for raw
    telemetry, with separate event and processing (ingest) timestamps.
*   CMake supports standard Linux builds and documents Android ARM64/Termux
    cross-compilation.

## 3. Current Project Status
*   Core telemetry domain model and time semantics are implemented and tested.

## 4. Task Tracker & Specs

### 📦 TO-DO (Active Specs)

*   *No active specs.*

### ✅ COMPLETED
* **Spec 1: Core Domain Model, Time Semantics & ARM64 Build**
