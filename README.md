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
*   *Setup phase. Blank slate.*

## 3. Current Project Status
*   *Repository initialized in Codespaces. Waiting for the first ingestion module.*

## 4. Task Tracker & Specs

### 📦 TO-DO (Active Specs)

* **Spec 1: Core Domain Model, Time Semantics & ARM64 Build**

* **Context:** Before opening network sockets or parsing data, we need a memory-safe data foundation that strictly respects DDIA Time Semantics and compiles for our Edge device.

* **Implementation:**
* Create a C++ struct/class `TelemetryPayload` representing the raw data: `device_id`, `event_timestamp_ms` (when the event occurred on the sensor), `gps_lat`, `gps_lon`, `ping_ms`.
* Add a separate field `ingest_timestamp_ms`. This must be populated by the system exactly when the struct is instantiated (Processing Time), cleanly separating it from `event_timestamp_ms`.

* **Constraints:**
* **Zero-Allocation Focus:** Avoid dynamic heap allocations for the payload. Do not use dynamically allocating `std::string` for `device_id`; use fixed-size `std::array` or `char` buffers to prepare the ground for zero-copy parsing.
* **Build System:** Create a `CMakeLists.txt`. It must build under standard Linux (x86_64) but include explicit comments/flags demonstrating readiness for ARM64 (Termux) cross-compilation.

* **Testing:**
* Integrate a lightweight testing framework (e.g., GoogleTest or Catch2).
* Write a unit test verifying the time semantics: ensure `event_timestamp_ms` is preserved exactly as input, and `ingest_timestamp_ms` is generated correctly.

### ✅ COMPLETED
*   *None.*