> **🛑 META-PROMPT FOR AI (NOTEBOOKLM - THE CHAOS ENGINEER & TEACHER):** 
> You are the Chaos Engineer. Your goal is to train the User in DDIA concepts (e.g., split-brain, network partitions, backpressure, LSM-trees).
> **Your Workflow:**
> 1. Read the "Incident & Test Tracker" below. 
> 2. If the last Incident is marked as "✅ PASSED" (meaning the Copilot successfully tested the User's solution), your job is to GENERATE A NEW INCIDENT.
> 3. Invent a physical, telecom-specific failure (e.g., "The Gateway loses connection to the OSS server for 5 minutes, but telemetry keeps arriving from the satellite at 10MB/s"). 
> 4. DO NOT provide the solution. Let the User write the Spec to fix it.

# Gateway Telemetry Shipper (Training Project)

## 1. Implemented Architecture
*   *Current State: Initial naive C++ daemon implemented. It accepts line-delimited telemetry over TCP, stores records in an in-memory append-only buffer, and forwards them to an OSS TCP endpoint with blocking I/O and reconnect backoff.*

---

## 2. Incident & Test Tracker (The Gradebook)
*(NotebookLM generates the Incidents. User writes the Specs. Copilot writes the Tests and updates the Status).*

### 🔴 ACTIVE INCIDENT
*   **Incident 01 (By NotebookLM):** The network link to the OSS Server drops completely. Telemetry from the LEO satellite continues to arrive at the Gateway at 10,000 metrics per second. 
*   **User Spec (The Architect's Fix):** 
    *   *Waiting for User to write the specification based on DDIA principles...*
*   **Chaos Test & Status:** 
    *   *Status:* 🚧 PENDING
    *   *Test Requirement:* Copilot must write a mock that simulates network drop and injects 50,000 metrics, asserting that no metrics are lost and memory does not infinitely balloon.

### ✅ RESOLVED INCIDENTS
*   *(Empty. Project just started).*

---

## 3. General Backlog (To-Do)
*   *(Empty for now. Awaiting next JIT specification.)*

## 4. General Backlog (Completed)
*   *Setup initial naive C++ daemon (Accepts telemetry, sends over TCP).*
