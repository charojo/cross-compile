# cross-compile

Embedded Cross-Platform Dev Workspace (i.MX8MP Cortex-A53)

## Contributing

Before making changes, please read [AGENTS.md](AGENTS.md) for code style,
commit message conventions, required tests, and expectations around keeping
`plan.md` and `tracking.md` up to date. Service-specific instructions live in
[cpp-service/AGENTS.md](cpp-service/AGENTS.md),
[python-worker/AGENTS.md](python-worker/AGENTS.md), and
[rust-agent/AGENTS.md](rust-agent/AGENTS.md).

## Continuous Integration

All pull requests run `make test` via GitHub Actions. The workflow
configuration lives in [`.github/workflows/ci.yml`](.github/workflows/ci.yml)
and sets up the toolchain inside a container before executing the tests.
Contributors should run `make test` locally and ensure the CI workflow passes
before opening or updating a pull request.

## Conclusion

This VS Code + DevContainer scaffold is designed to enable a team to quickly start developing cross-platform (C++/Python/Rust) services for the i.MX8MP while ensuring alignment with the **Option A architecture** recommendations (ZeroMQ messaging, Protobuf schemas, SQLite persistence, etc.). It provides a comprehensive environment where everything from writing code, building, running on an emulated target, debugging, benchmarking, and testing is streamlined and reproducible.

By using modern containerized development practices and proven OSS components, we achieve a **clean, low-friction workflow** that emphasizes clarity and consistency. New developers can onboard with minimal setup, and the documentation (of which this report is a part) serves as both an onboarding guide and a technical reference for the system’s design and rationale. The emphasis on performance testing, CI integration, and traceability ensures that as the project grows, it remains maintainable and verifiable.

Moving forward, this scaffold can be extended with more services or tools as needed (e.g., adding a web UI container, or integrating a simulator for hardware inputs). Its modular nature and adherence to standards mean such additions can be made without restructuring the whole environment. Developers and stakeholders can be confident that this foundation will scale with the project’s needs, all while maintaining the **core principles of real-time, reliable, cross-language communication** that are essential for the product’s success.

## Project Plan

The overall roadmap and milestone breakdown live in [project/plan.md](project/plan.md).
Current feature status and links to issues or pull requests are tracked in [project/tracking.md](project/tracking.md).

## Architecture and Technology Stack Overview

To support **embedded cross-platform development** on the NXP i.MX8MP (Cortex-A53) platform, this scaffold follows the proven **Option A architecture** combining **ZeroMQ**, **Protobuf**, **SQLite**, and real-time messaging principles. In this design, a central **Data Service** coordinates state with an in-memory cache and persistent SQLite storage, while multiple microservices (in C++, Python, Rust) communicate via a **ZeroMQ message bus** using **Protocol Buffers** for schema-defined data. This approach provides low-latency IPC (REQ/REP for requests and PUB/SUB for event broadcasting), strongly-typed cross-language messages, and lightweight embeddable persistence. Real-time pub/sub messaging ensures that updates propagate to subscribers immediately, aligning with the platform goal of **real-time data sharing and updates across independent services**. Dynamic service discovery (via **Zyre** library) is included for peer discovery without central brokers, reflecting the requirement for modular extensibility in distributed deployments. Overall, this technology stack is chosen for its **high performance, cross-platform interoperability, and small footprint**, making it ideal for embedded Linux/Android environments.

**Key Components:**

* **ZeroMQ Message Bus:** Acts as the inter-process communication backbone with support for patterns like publish/subscribe and request/reply. ZeroMQ provides *elastic, high-throughput messaging* that is asynchronous and broker-less. Messages are passed as binary blobs; we leverage Protobuf for serialization on top of ZeroMQ. This yields a flexible, real-time event system (e.g. services publish updates that UI clients subscribe to) with minimal latency.

* **Google Protobuf Schemas:** Define the structure of messages exchanged between services (for example, an `UpdateUserRequest` and `UpdateUserResponse` as shown below). Protobuf ensures *schema-defined, versionable data exchange* that is language-agnostic. All services use generated code (C++, Python, Rust) from the common `.proto` definitions to guarantee consistency. *(Example schema)*:

  ```proto
  syntax = "proto3";
  message UpdateUserRequest { int32 user_id = 1; string field = 2; string value = 3; }
  message UpdateUserResponse { bool success = 1; string message = 2; }
  ```

* **SQLite Storage:** Provides a lightweight, embeddable database for persistent storage. The C++ data service uses SQLite to durably save state changes (with a simple **write-through cache** strategy: update in-memory cache and asynchronously commit to SQLite). SQLite's reliability and zero-configuration nature suit offline-capable embedded devices. It also has a small footprint, aligning with code size constraints for embedded fit.

* **In-Memory Cache:** A shared memory cache (within the Data Service process) holds hot data for fast read access by services. Writes go through the Data Service, updating the cache and database in tandem to ensure consistency. This design yields high-speed reads without duplicating large state in every service. (In future, a distributed cache like Redis could be swapped in, but a local in-memory cache ensures simplicity and no additional moving parts).

* **Zyre Discovery:** For dynamic peer discovery, the scaffold integrates Zyre (built on ZeroMQ) to allow services to announce themselves and discover others on the network without a central registry. This suits scenarios where components can appear/disappear (e.g. USB peripherals or optional modules) and need to find each other at runtime. The dev environment can include Zyre for testing discovery in a local setting.

This architecture balances performance and portability: *ZeroMQ + Protobuf* offers fast, cross-language messaging, and *SQLite + Cache* provides reliable persistence and quick data access. It scored highest in earlier feasibility studies for meeting embedded requirements (low latency, offline capability, small footprint).

## Repository Structure and Starter Examples

The repository is structured to showcase **modular starter examples** in C++, Python, and Rust, each representing a component in the distributed system. All source code is included with extensive comments (and listed in the documentation for easy reference), so developers can use these as templates for new services. The high-level layout is:

```
.
├── proto/                # Protobuf schemas (.proto files) for messages
├── cpp-service/          # C++ Data Service example (ZeroMQ + Protobuf + SQLite)
├── python-worker/        # Python 3 Worker example (subscriber/client interface)
├── rust-agent/           # Rust microservice template (IPC bindings via ZeroMQ/Protobuf)
├── shared/               # Shared libraries or utilities (e.g. common protos, utils)
├── .devcontainer/        # DevContainer config (Dockerfile, devcontainer.json)
├── .vscode/              # VS Code tasks and launch configurations
├── docker-compose.yml    # Docker Compose setup for dev environment
├── Makefile              # Makefile with common targets
└── docs/                 # Additional documentation (including this design report)
```

### C++ Data Service (`cpp-service/`)

**Role:** The C++ service acts as the **Data Service** – a central authority for storing and distributing data. It hosts a ZeroMQ **REP** socket to handle incoming requests (e.g. update or query data) and a **PUB** socket to broadcast change notifications. It uses the SQLite C++ API (e.g. `sqlite3_exec`) to persist updates to an on-disk database, while maintaining an in-memory cache (e.g. an `std::unordered_map` or similar) for fast reads.

**Technologies:** C++17 (or later), ZeroMQ (via \<zmq.hpp>), Google Protobuf (via generated C++ classes), SQLite (via \<sqlite3.h>). The build uses **CMake** with toolchain settings for cross-compiling to ARM64.

**Key Code Components:**

* `DataService.cpp` – Implements a simple event loop: uses `zmq::socket_t rep(ctx, ZMQ_REP)` to receive request messages, parses them using Protobuf (`UpdateUserRequest` from `data.pb.h`), performs the requested operation (update the cache and DB, or fetch data), then sends a Protobuf-encoded reply via REP. After processing a mutation, it also publishes a notification on a PUB socket (e.g. an `UpdateUserEvent` message).

* `Cache.h/Cache.cpp` – A simple in-memory cache interface (e.g. `get(user_id) -> UserData` and `put(user_id, UserData)` methods). In this demo, it could just wrap an STL map. The DataService updates the cache on writes and checks it on reads to avoid hitting the DB for recent data.

* `Database.cpp` – SQLite wrapper handling initialization and CRUD operations. For example, uses `sqlite3_open` to open a DB file in the container (persisted under a volume), and provides methods like `updateUser(user_id, field, value)` and `getUser(user_id)` that execute parameterized SQL queries. On updates, it writes through from cache to DB to ensure durability.

* `ProtoBuf Integration:` The Protobuf schema (in `proto/data.proto`) is compiled to C++ (`data.pb.cc`), which the service uses to deserialize requests and serialize responses. For instance, the REP handler does:

  ```cpp
  zmq::message_t reqMsg;
  socket_rep.recv(reqMsg);
  UpdateUserRequest req;
  req.ParseFromArray(reqMsg.data(), reqMsg.size());
  // ... process request ...
  UpdateUserResponse resp;
  resp.set_success(true);
  resp.set_message("User updated");
  std::string respData = resp.SerializeAsString();
  socket_rep.send(zmq::buffer(respData));
  ```

* `ZeroMQ Patterns:` The service binds its REP socket (e.g. at `tcp://0.0.0.0:5555`) and PUB socket (e.g. `tcp://0.0.0.0:5556`). Other components then connect to these endpoints. ZeroMQ’s asynchronous I/O means the service can handle incoming requests and outgoing pub messages concurrently using a polling loop or multi-threading as needed. This example keeps it simple with a single-threaded loop using `zmq_poll()` on both sockets.

**Build & Run:** The C++ service is built into an ARM64 binary (`data_service`) by the cross-toolchain. In the dev container, you can run it under QEMU to test (e.g. `qemu-aarch64 -L /usr/aarch64-linux-gnu/ ./build/data_service`). It will create or use an SQLite DB file (on a mounted volume, so persisted). Clients can send it requests to update or fetch data, and it will log actions to stdout for observability.

### Python Worker (`python-worker/`)

**Role:** The Python example simulates a background **Worker/Subscriber** service. It subscribes to data change events and could perform additional processing (e.g. updating a search index or triggering external calls) upon receiving notifications. It can also act as a client to request data or send commands to the Data Service.

**Technologies:** Python 3 (3.9+), `pyzmq` (ZeroMQ bindings), `protobuf` library (for Python, using generated classes from the same `.proto`), and possibly `sqlite3` for any local use (though primarily the Python relies on the central Data Service for data).

**Behavior:** On startup, the worker connects a SUB socket to the Data Service’s PUB endpoint (e.g. `tcp://data_service:5556` within the Docker network or `localhost` if all in one container) and subscribes to relevant topics (e.g. all updates or a subset like `user_updates`). It also has a REQ client socket to the Data Service’s REP endpoint for on-demand queries. The script listens in a loop using `poller.poll()`. When an update message arrives, it deserializes it using the Python Protobuf class (e.g. `UpdateUserEvent.ParseFromString(msg)`), then logs it or triggers an application-specific action (for demo, perhaps just prints a message indicating it received the update). The worker can also periodically send a request (using REQ) to fetch some data (e.g. ask for a user record) to demonstrate bidirectional interaction.

**Example Snippet:**

```py
import zmq, data_pb2
ctx = zmq.Context()
sub = ctx.socket(zmq.SUB)
sub.connect("tcp://data_service:5556")
sub.setsockopt(zmq.SUBSCRIBE, b"")  # subscribe to all topics
req = ctx.socket(zmq.REQ)
req.connect("tcp://data_service:5555")

# Listen for updates in background
while True:
    topic, msg = sub.recv_multipart()  # assuming pub sends multipart [topic][data]
    event = data_pb2.UpdateUserEvent.FromString(msg)
    print(f"Received update: user {event.user_id} changed {event.field}")

    # Example: request full user record on each update
    req.send(data_pb2.GetUserRequest(user_id=event.user_id).SerializeToString())
    resp_msg = req.recv()
    resp = data_pb2.GetUserResponse.FromString(resp_msg)
    print(f"Fetched user data: {resp.user}")
```

This worker demonstrates Python integration in the system: it uses the same Protobuf definitions to interoperate with the C++ service. Python’s ease of use makes it ideal for tasks like data analytics or orchestration in the embedded platform.

**Note:** In the dev container, the Python worker runs on x86 (since it’s Python bytecode) but still connects to the Data Service running under QEMU. Alternatively, one can also cross-compile Python code or use it on target with an ARM Python interpreter if needed. Here we focus on using it as a development-time component.

### Rust Microservice (`rust-agent/`)

**Role:** The Rust example is a template for a **microservice (Sensor Agent)** that could be written in Rust. It interfaces with the same message bus and protocols, showcasing that any language with ZeroMQ and Protobuf support can participate. Rust is chosen for its reliability and performance, ideal for tasks like sensor data handling or computation-heavy services.

**Technologies:** Rust (edition 2021), `zmq` crate (Rust bindings for ZeroMQ), `prost` or `protobuf` crate for Protobuf support (prost is a popular choice for Rust Protobuf implementation). The project is a standard Cargo project, cross-compiled to AArch64-unknown-linux-gnu target.

**Behavior:** The Rust agent might simulate a sensor that publishes readings periodically. For instance, it could gather a mock value (e.g. temperature) and publish a message every second. It might also listen (SUB) for control messages (e.g. a command to change sampling rate). This demonstrates bi-directional communication in a Rust service.

**Example Implementation Highlights:**

* Uses `let ctx = zmq::Context::new();` and creates a **PUB socket** (e.g. bound to `tcp://0.0.0.0:5557` or connects to a central bus) to publish sensor readings, and a **SUB socket** to subscribe to a control channel (if needed).
* Defines Protobuf message types in Rust via `prost_build` in build script or includes generated code. For instance, a `SensorReading` message with fields like `sensor_id` and `value`. The Rust code packs data into this message and then publishes the serialized bytes.
* Implements a simple loop with `std::thread::sleep()` to periodically send messages. Each message is tagged with a topic if using PUB/SUB (ZeroMQ allows sending topic as first frame).
* Error handling and logging with Rust’s `log` crate (e.g. using env\_logger or similar). The scaffold configures the Rust example with logging to stdout for integration with other components’ logs.

By providing a Rust template, we ensure the scaffold is truly cross-language. Developers can use it as a starting point to build more complex Rust-based services that still seamlessly interact with C++ and Python components over the common bus. This demonstrates interoperability: for example, the Rust agent’s published Protobuf messages can be parsed by the Python worker or C++ service, proving end-to-end compatibility.

**Building & Testing:** The Rust project includes a `Cargo.toml` with cross-compilation settings for `aarch64-unknown-linux-gnu`. In the dev container, environment variables (like `CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER`) are set to use the cross GCC. Running `cargo build --target=aarch64-unknown-linux-gnu` produces an ARM binary which can be run via QEMU. A small test (e.g. using `cargo test`) can also run in QEMU user mode if needed for verification.

### Full Source Listings and Code Documentation

All the above examples are thoroughly documented in-code, and their **full source code is included in the repository and appendices of the documentation**. New developers are encouraged to read through these implementations to understand how cross-language messaging, serialization, and data management are handled. By studying the provided code (and running it in the contained environment), developers will gain practical insight into the architecture’s **real-time messaging and data flow**. This sets a solid foundation before writing new services or modifying existing ones.

*(Refer to the appendix or the `docs/` folder for complete source listings of each example component, as well as the generated Protobuf code. The documentation also cross-references these examples with the architecture concepts they implement.)*

## Cross-Compilation Toolchain and Build Configuration

To target the ARM Cortex-A53 (64-bit ARMv8) of the i.MX8MP, the development environment includes a **full cross-compilation toolchain** for AArch64. All C/C++ and Rust code is compiled on the x86\_64 host (in Docker) but producing binaries for **aarch64-linux-gnu** (the 64-bit ARM Linux platform). Key aspects of the toolchain support:

* **GCC Cross-Compiler:** The dev container installs the GNU Arm toolchain (e.g. `gcc-aarch64-linux-gnu` and related binutils). This corresponds to the **aarch64-linux-gnu** target triple, appropriate for 64-bit Linux on Cortex-A53. The compiler is invoked as `aarch64-linux-gnu-gcc`/`g++`, and is configured to produce code for ARMv8-A architecture. We set `-mcpu=cortex-a53` (and `-mtune=cortex-a53` if needed) to optimize for the specific core, and use `-march=armv8-a`. Floating point and NEON are enabled by default for this core (A53 has NEON). The toolchain also links against ARM versions of standard libraries (provided via a sysroot).
* **Multi-lib and Sysroot:** The container uses Debian multi-architecture support to install ARM64 versions of needed libraries (ZeroMQ, SQLite, etc.) in `/usr/aarch64-linux-gnu/`. This serves as the **sysroot** for cross-compilation and for QEMU to find libraries. For example, `libzmq.so` and `libsqlite3.so` for arm64 are installed so that the cross-compiled binaries can link against them. CMake toolchain file or environment variables (`CMAKE_SYSROOT`) point to this location ensuring the linker finds the correct libs.
* **Yocto SDK Integration (Optional):** In some cases, the target device might have a Yocto-generated SDK (e.g. from NXP’s BSP) for exact library alignment. The scaffold provides a mechanism to integrate such an SDK. If a Yocto SDK is available, developers can **source an environment script** (provided by Yocto) inside the dev container which sets environment variables (`CC`, `CXX`, `SDKTARGETSYSROOT`, etc.) for that toolchain. We include wrapper scripts (e.g. `source yocto-env.sh`) that *override the default GCC with the Yocto-provided compiler and sysroot*. The build system is designed to detect this and use the Yocto toolchain if activated. This ensures maximum compatibility with the target OS (library versions, libc, etc.), while still allowing the simplicity of the Debian cross-compiler by default.
* **Clean Environment Wrappers:** To avoid the notorious “PATH soup” issue (mixing host and target tools in PATH), the dev container and build scripts enforce a **clean build environment**. For instance:

  * The container’s PATH is set such that the cross-toolchain comes first and host compilers are either absent or not in PATH. We **do not** rely on the developer’s host PATH at all.
  * CMake toolchain files explicitly set compilers and file search paths, so finding headers/libraries is confined to the sysroot and avoids accidentally picking up x86 libraries.
  * The Makefile’s `configure` target (see below) calls CMake with a fresh environment, and our wrapper `dev_env.sh` can be used to enter a subshell with all needed env variables (this helps if developers want to run cross-compiler by hand).
* **CMake Presets for Profiles:** We leverage **CMakePresets.json** to define various build configurations systematically. There are presets for common scenarios:

  * `debug` – Debug build (no optimizations, `-O0`, with debug symbols).
  * `release` – Release build with optimizations (`-O3`) for maximum speed.
  * `minsize` – Release with size optimizations (`-Os`) for smaller binary footprint.
  * `release-lto` – Release with Link-Time Optimization enabled (`-O3 -flto`) for additional performance.
  * `release-pgo` – Profile-Guided Optimization build (two-stage build using `-fprofile-generate` then `-fprofile-use`). This one is handled via the bench harness (explained later) since it requires running the instrumented binaries to gather profiles.
  * Each preset sets the appropriate compiler flags and linker flags. For LTO, we ensure CMake sets `CMAKE_INTERPROCEDURAL_OPTIMIZATION=TRUE` (supported since CMake 3.16) to enable LTO automatically. For PGO, the preset might set `CMAKE_CXX_FLAGS="-fprofile-generate"` for the generation phase; the bench script or CI would then handle the profile use phase.
  * The presets also specify the *toolchain file* for cross-compiling or set the `CMAKE_SYSTEM_NAME` and compilers to the cross ones. This means developers can configure the project with one command (or VS Code CMake Tools integration) by selecting the preset, without manually typing out all flags.
* **Optimizations (LTO/PGO):** By toggling these profiles, developers can easily build variants and measure their effect. LTO can significantly reduce code size and sometimes improve speed by cross-module optimizations. PGO can yield tailored optimizations based on runtime profile data. The scaffold encourages experimentation with these flags to achieve the best trade-off for the embedded deployment. Our documentation gives guidance on using these (e.g. run `make bench` to see performance with each). Note that PGO requires running on representative inputs – our bench harness automates this by running a standard workload on QEMU to gather profile data, then rebuilding with `-fprofile-use`.
* **Rust Toolchain:** For Rust, we install the Rust compiler (rustup toolchain) and add the `aarch64-unknown-linux-gnu` target. Cargo is configured to use the cross linker (`aarch64-linux-gnu-gcc`). This is handled either by a `.cargo/config.toml` in `rust-agent/` or by environment variables set in the devcontainer. Rust code can thus be cross-compiled seamlessly. (We also ensure that any needed C libraries for Rust (like if using `openssl` or other native deps) are available for ARM in the sysroot).
* **Ensuring Reproducibility:** All compiler versions and dependencies are pinned via the devcontainer image, so every developer uses the *same tool versions*. This avoids "works on my machine" problems. Even the **Protobuf compiler** (`protoc`) is installed in a specific version and used to generate code, ensuring consistent schema code across languages.

With this toolchain setup, developers can build the entire project for the target architecture *with one command* in the container, without manually managing cross-compilers. The complexity of cross-compiling (proper flags, environment, etc.) is encapsulated in the configuration, allowing developers to focus on code.

## Continuous Benchmarking with QEMU

Performance and memory footprint are critical in embedded projects. This scaffold includes a **QEMU-based benchmarking harness** that automates building and testing different build profiles on the emulated target CPU. The goal is to provide quantitative feedback on how each optimization (O2 vs O3 vs Os, LTO, PGO, etc.) impacts **runtime performance**, **binary size**, and **memory usage**.

**Benchmark Harness (`bench.py` or make bench):** A Python or shell script orchestrates the following steps (triggered by `make bench`):

1. **Build All Profiles:** It iterates over a set of predefined build presets (e.g. `release`, `minsize`, `release-lto`, `release-pgo`). For each, it configures and builds the project (possibly in separate build directories to avoid flag interference). This can be done via CMake presets invocation or by directly calling `cmake -DPROFILE=X ...` etc. The result is a set of binaries for each configuration.
2. **Run Under QEMU:** For each built binary (or a representative subset of them, e.g. the main service binaries), the script uses QEMU user-mode to execute it on the host. We use **qemu-aarch64** (since target is ARM64) with the appropriate `-L` option to point to the target sysroot so that the program uses the correct ARM libraries. For example: `qemu-aarch64 -L /usr/aarch64-linux-gnu ./build/release/data_service --benchmark` might run a built-in benchmark mode of the service. If the program requires arguments or an input dataset, the script provides a consistent input (the scaffold could include a dummy workload, e.g. processing a set of 1000 messages).

   * We ensure QEMU is installed in the container and configured via binfmt\_misc so it can be invoked seamlessly. If not using binfmt, the script explicitly calls qemu with the `-L` flag.
3. **Measure Runtime and RSS:** We wrap the execution with the GNU `time` command (or Python’s `subprocess` + `/proc` reading) to capture the execution time and peak memory. For instance: `/usr/bin/time -f "%e,%M" -o time.txt qemu-aarch64 -L ... <binary>`. This gives the runtime in seconds and max resident set size in KB. We also record CPU usage if relevant, though with QEMU it’s virtual CPU time.
4. **Measure Binary Size:** The script records the output binary’s file size on disk (and can also use the `size` tool to get text/data segment sizes). This helps quantify code bloat vs optimization.
5. **Collect Results:** For each profile, the results (runtime, memory, binary size) are collected into a CSV file (`bench_results.csv`) with columns like `profile, runtime_s, max_RSS_KB, binary_size_KB`. This file allows easy analysis or plotting over time.
6. **Generate Report:** The harness then generates a human-friendly **`report.html`**. This can be as simple as an HTML file with a table of results and some highlights, or more sophisticated with charts (maybe using a Python library like matplotlib or plotly to embed a graph comparing profiles). For example, a bar chart of runtime vs binary size for O3 vs Os vs LTO might be shown. The report summarizes which profile is fastest, which is smallest, etc., guiding developers in choosing a build mode for production. If PGO was run, its results typically show the best performance, which can justify the extra complexity for final builds.
7. **Profile-Guided Optimization integration:** For the PGO profile, the harness will perform a two-step process:

   * First compile with `-fprofile-generate` and run the target to produce a profile data file (e.g. `.gcda` files). QEMU is used to execute the instrumented binary on a sample workload.
   * Then recompile with `-fprofile-use` and measure the performance of the optimized binary. This automates the otherwise manual PGO process.
8. **Automation and CI:** The entire bench process can be run manually by a developer, but it’s also integrated into CI (as a separate job, perhaps not on every commit but nightly or on-demand) to track performance over time. The CSV results could even be archived or compared to detect regressions.

Running the benchmark is as easy as: `make bench`. After it completes, developers can open `report.html` (the devcontainer includes a simple web server or they can use the VS Code GUI to view it) to see the results. This immediate feedback loop encourages performance-aware development.

**Example Results:** After running, the `bench_results.csv` might look like:

```
profile, binary_size_kb, runtime_s, max_rss_kb
O3,       450,           1.25,      5120
Os,       320,           1.40,      4900
LTO,      400,           1.20,      5130
PGO,      450,           1.10,      5120
```

And the HTML report will present this in a table and note that PGO achieved \~12% faster runtime than baseline O3, while Os saved \~30% size at cost of 12% slower runtime, etc. Such data-driven insights help in making informed trade-off decisions.

**QEMU Accuracy:** While QEMU user-mode is not cycle-accurate, it provides a reasonable approximation of code performance trends and allows running the ARM binaries quickly on the host. It’s extremely useful for development testing (unit tests, etc., as well) because it avoids needing actual hardware for each test run. Memory alignment and endianness issues can be caught with QEMU in user-mode, as it executes the actual ARM instructions. For full-system integration tests or more accurate perf analysis, one could use QEMU system-mode with a virtual ARM Linux, but that’s beyond scope for everyday dev workflow (and slower). The chosen approach maximizes speed and convenience.

In summary, the QEMU-based benchmarking ensures that performance optimizations are not theoretical – developers can empirically verify the impact of their choices on the target architecture early and often. It also doubles as a quick way to **unit test** the cross-compiled binaries in the CI pipeline without physical hardware.

## Dev Environment: Docker Compose & DevContainer Setup

To streamline setup, the project uses a **VS Code DevContainer** configuration with **Docker Compose**. This encapsulates all dependencies and tools in a container, providing a consistent and reproducible environment for all developers. With Docker Compose, even single-container setups benefit from easy volume management and extensibility.

### Docker Image and Tools

The `.devcontainer/` directory contains a Dockerfile that defines the development image. Key components installed in this image:

* **Base OS:** Debian 12 (Bookworm) or Ubuntu 22.04, chosen for broad ARM cross-toolchain support.
* **Cross Toolchain:** as discussed, `gcc/g++ aarch64-linux-gnu`, `gdb-multiarch`, binutils for ARM, QEMU user static binaries (for aarch64).
* **Languages & Runtimes:** Python 3 (with pip) for running scripts and the Python example; Rust (via rustup or distribution) with target installed; Node.js can be added if needed for any tooling (not specifically required here).
* **Libraries:** libzmq3-dev, libsqlite3-dev for both host (x86\_64, for building any host tools or running natively) and for arm64 (cross libraries). Protobuf compiler (`protoc`) and libprotobuf-dev for generating code. Zyre library and dependencies (built or installed if available in apt).
* **CMake & Build Tools:** CMake (>=3.20 for presets support), Ninja (optional, used as the generator for faster builds), Make (for compatibility), and build-essential for native compilation when needed.
* **Helper Tools:** Git (for VCS), ccache (enabled to speed up repeated builds, configured to store cache in a volume), Doxygen or other documentation generators if needed, and Ansible (if using it for IT setup scripts).

**DevContainer Configuration:** The `devcontainer.json` references the Dockerfile and also sets up VS Code specifics:

* It mounts the source code into `/workspace` (the default).
* It specifies the `docker-compose.yml` to use. Our **docker-compose.yml** defines a service `dev` (the development container) with:

  * Build context pointing to the Dockerfile.
  * Volume mounts:

    * `.:/workspace:rw,z` bind-mounts the project source (so edits on host reflect inside, though typically you’d edit from VS Code which is inside container but syncing to host).
    * Named volumes for build artifacts: e.g. `build_cache:/workspace/build`, `ccache_cache:/home/vscode/.ccache`, `cargo_target:/workspace/rust-agent/target`. These named volumes persist data between container runs. They prevent the host filesystem from being cluttered with build files and also improve performance (especially on Windows/Mac where file I/O is slower on bind mounts). For example, `ccache_cache` ensures that object cache is saved even if container is rebuilt, and `cargo_target` holds compiled Rust dependencies so they don’t need re-downloading every time.
    * Another volume `vscode_extensions:/home/vscode/.vscode-server/extensions` might be used so VS Code extensions don’t re-install each time (optional).
  * The container runs as a non-root user (e.g. `vscode`) to match typical devcontainer practice, with the UID/GID possibly matching the host user for permission ease.
  * Device mappings or privileged flags: Usually not needed for QEMU user mode, but if using QEMU system mode or USB access, one might add `privileged: true` or specific device mounts. For now, not required.
* **Extensions & Settings:** devcontainer.json recommends VS Code extensions such as:

  * C/C++ Extension (ms-vscode.cpptools) for IntelliSense and debugging.
  * Python extension (ms-python.python) for Python IntelliSense and debugging.
  * Rust Analyzer (rust-lang.rust-analyzer) for Rust language support.
  * YAML, Ansible extensions if writing playbooks.
  * Possibly a Dev Containers extension itself (though host needs it).
    It also sets some settings, e.g., the C\_Cpp default debugger to `gdb-multiarch` and configures the IntelliSense to use compile\_commands from the CMake build or to use the `gcc-arm` target includes.
* **Startup:** On first launch, the container runs a post-create script: e.g. `apt update && apt install additional-tools` if needed, or a script to fetch certain large dependencies (but we try to bake most into the image). It might also run `cmake --preset debug` to prime the build directory, so that IntelliSense can pick up headers.

With this setup, any developer with Docker and VS Code can get started by simply opening the repository in VS Code and consenting to “Reopen in Container”. Within minutes, they have a fully-configured environment identical to everyone else’s.

### Using Docker Compose

We opted for Docker Compose for configuration, even though it’s a single dev container, because **Compose YAML makes it easy to manage volumes and environment variables in a declarative way**. It also sets the stage for multi-container setups (for example, if we later add a separate container to simulate the target device or a database service, we can extend the compose file). The compose file defines named volumes as described and ensures the container has access to the X11 display or other resources if needed (for GUI, though here likely headless).

To bring up the dev environment without VS Code, a developer can simply run: `docker-compose up -d dev` followed by `docker-compose exec dev bash` to get a shell. This is documented for those who prefer CLI or are using different editors. The container will have everything set up (source mounted, tools installed), so from that shell one can run builds or tests manually.

### Environment Isolation and Reproducibility

By containerizing the dev environment, we eliminate differences like OS-specific tool versions. For example, whether the developer is on macOS, Windows (WSL2), or Linux, they all build using the *same* cross-compiler and libraries inside the container. This significantly reduces onboarding friction: no need to manually install a bunch of toolchains or worry about conflicting versions. It also means the **CI can use the same container** to ensure builds are identical to local builds.

The environment is also disposable – if something goes wrong, one can rebuild the container from scratch and be assured a clean slate. This avoids "pollution" of the host OS with compilers and SDKs.

**Performance considerations:** Using a container means builds run in a VM (on Windows/macOS), but thanks to ccache and volume caching, repetitive builds are fast. The first build of large projects might be somewhat slower than native, but the convenience trade-off is usually worth it. For editing, VS Code’s remote container extension streams file changes efficiently; building inside container is nearly as quick as native after initial setup.

### Running and Debugging in Container

The devcontainer is configured to facilitate easy **building, running, and debugging** of the target software:

* The **ZeroMQ endpoints** in examples are configured to work within the container network. For instance, the Python worker connects to `data_service` host which can be an alias in compose if the Data Service runs in another container, or just `localhost` if running within the same container. We note these in documentation so developers know how to adjust if running across multiple containers or devices.
* **GDB debugging:** We installed `gdb-multiarch` which is capable of debugging ARM executables on an x86 host. The VS Code launch configuration uses this to debug programs under QEMU:

  * We use a compound launch: first a task runs `qemu-aarch64 -L /usr/aarch64-linux-gnu -g 12345 ./build/debug/data_service` (for example) which starts the program and halts waiting for a debugger on port 12345. Then VS Code attaches with gdb-multiarch to `localhost:12345`. This allows full debugging of the ARM binary as if we were on target – one can set breakpoints, inspect variables, etc., all through the familiar VS Code debug UI. We also ensure to tell GDB the sysroot to locate shared libs (`set sysroot /usr/aarch64-linux-gnu`).
  * The launch configurations (`.vscode/launch.json`) include pre-configured debug setups for each example binary (C++ service, maybe Rust agent if needed). Developers can hit F5 to launch the debug session. Thanks to the integration of QEMU’s GDB stub, the experience is quite seamless.
* **Hot-reload and Testing:** For Python code, VS Code can directly run and debug it (since it’s running on x86 Python in the container). For Rust, debugging cross-compiled code can also be done via gdb (Rust symbols, etc., are available – rust-gdb might not be needed since gdb-multiarch suffices). We also support running unit tests in the container:

  * C++ tests (if any, using GoogleTest or similar) can be cross-compiled and executed under QEMU with a tasks or CTest integration.
  * Python tests can run natively (with coverage, etc).
  * Rust tests for cross target are trickier (you can cross-compile tests and use QEMU to run them – the harness can do that).
  * We provide tasks like `test:cpp`, `test:rust`, `test:all` in tasks.json to automate these steps.

Overall, the combination of DevContainer and Docker Compose ensures a **low-friction development workflow**: check out code, open in container, and everything just works. Developers spend time writing code, not fighting environment setup.

## Build System and Makefile Usage

While CMake and Cargo handle most of the build process, we provide a top-level **Makefile** as a user-friendly interface to common actions. This Makefile is essentially a wrapper around the underlying build systems, consolidating commands for convenience and documentation (by running `make help`). It includes the following targets:

* **`make help`** – Lists all available targets with a brief description. This is the default target, so running `make` with no arguments will show the help info. It ensures newcomers can discover the build options quickly.
* **`make configure`** – Configures the C++ CMake project for the cross build. This invokes CMake (possibly via `cmake --preset`) to set up the build directory. For example, `cmake -S cpp-service -B build/cpp-service -DCMAKE_TOOLCHAIN_FILE=... -DCMAKE_BUILD_TYPE=Debug` (or uses a preset like `aarch64-debug`). It also generates compile\_commands.json for IDE integration. If a Yocto SDK is being used, this target will detect it (e.g., via env var `YOCTO_SDK_ROOT`) and include the appropriate toolchain definitions. Essentially, this target prepares everything so that `make build` can proceed.
* **`make build`** – Builds all components (C++, Rust, etc.) in release mode by default. It calls `cmake --build build/cpp-service --target all --config Release` and also invokes `cargo build --release --target aarch64-unknown-linux-gnu` for the Rust project, and ensures any Python bytecode or preparation is done (though Python is interpreted, we might compile the Protobuf Python stubs). There could be sub-targets like `build-cpp`, `build-rust` if needed, but `build` covers everything. Developers can override the build type by `BUILD_TYPE` variable or use separate targets (e.g., `make debug` for a debug build).
* **`make run`** – (If applicable) Launches the main services in a dev/test mode. For instance, could start the Data Service under QEMU and maybe also launch the Python worker. This might be implemented as a docker-compose override or simply using QEMU and python directly. It’s mainly for quick local testing. E.g., `qemu-aarch64 -L /usr/aarch64-linux-gnu ./build/cpp-service/data_service & python3 python-worker/worker.py`.
* **`make bench`** – Runs the benchmark harness described earlier. After building necessary profiles, it executes the tests under QEMU and generates the CSV/HTML results. It prints a summary to console and tells where to find the full report. This target ensures performance testing is just one command away.
* **`make report`** – Generates or opens the report. If the bench target already produced `report.html`, this target might simply open it (within VS Code or via `xdg-open` if using an X11 setup). Alternatively, it could regenerate documentation (if we had a documentation generation step like Doxygen or Sphinx). Here, likely it’s tied to the benchmark/coverage report generation.
* **`make clean`** – Cleans up build artifacts. This removes the `build/` directory (except perhaps leaves ccache intact). It also has suboptions like `make clean-cpp`, `clean-rust` if needed. Essentially, it allows starting fresh. Also could prune the ccache and other caches if `make distclean` is invoked.
* **Phony targets for convenience:** e.g. `make cpp` to build only the C++ portion, `make rust` to build Rust, etc., depending on team preferences.
* The Makefile is careful to use the container’s environment (so if run on host outside container, it might detect that and warn to use the container). Ideally, developers run these inside the dev container where all tools are present.

We also integrate the Makefile with VS Code tasks, so running a build task in VS Code essentially calls `make build` inside the container.

The presence of a Makefile offers a gentle learning curve for those used to Make or simply as documentation of how to use the project. It encapsulates the multi-step processes (especially for newcomers not familiar with CMake presets or cross-compiling intricacies).

Example excerpt from the Makefile (for illustration):

```Makefile
.PHONY: help configure build bench clean

help:
\t@echo "Available targets:"
\t@echo "  configure - Configure CMake for target platform"
\t@echo "  build     - Build all components (C++, Rust) in release mode"
\t@echo "  bench     - Run benchmarks on QEMU and generate report"
\t@echo "  clean     - Remove build artifacts"

configure:
\tcmake -S cpp-service -B build/cpp-service --preset aarch64-release

build: configure
\tcmake --build build/cpp-service --parallel
\tcargo build --target=aarch64-unknown-linux-gnu --release

bench: build
\tpython3 scripts/bench.py

clean:
\trm -rf build/ && cargo clean
```

(Actual implementation may be more robust with conditionals and detection.)

## Developer Onboarding and Workflow Guide

This section serves as a quick-start guide for developers on different host environments and outlines key practices (including IT setup steps, CI, and quality assurance).

### Setup Instructions by Host Platform

**For Linux Developers:**

1. **Install Docker:** Ensure Docker Engine is installed and you have permission to run Docker (e.g. your user is in the `docker` group). Docker Compose (V2) is typically included.
2. **Install VS Code:** Have Visual Studio Code installed, along with the *Dev Containers* extension (ms-vscode-remote.remote-containers).
3. **Clone Repository:** `git clone <repo_url>` and open the folder in VS Code.
4. **Open in Container:** VS Code should prompt “Reopen in Container”. Do so. This will build the container image (first time, it could take several minutes to pull base images and install packages) and start the container.
5. **Develop:** Once inside, you can build the project by invoking the VS Code Terminal (which is inside container) or using the provided tasks (Ctrl+Shift+B to run build task).
6. **Run & Debug:** Use the debug panel to launch the Data Service or other targets in QEMU. Set breakpoints in C++ or Rust code; use Python debugging for the worker script.
7. **Iteration:** Edit code and rebuild as needed. Thanks to ccache and incremental builds, subsequent builds are faster.
8. **Stop:** When done, you can simply close VS Code. The container may stop automatically, but you can also run `docker-compose down` to explicitly stop it. Your source and named volumes persist.

**For Windows Developers (via WSL2 or Docker Desktop):**

* If using **WSL2**: It’s recommended to clone the repo inside the Linux filesystem (e.g., in your Ubuntu home directory) for performance. Then open VS Code from WSL (using the Remote WSL extension). From within the WSL VS Code session, launch the Dev Container. The experience thereafter is the same as Linux. The container will run in WSL2 backend.
* If using **Docker Desktop on Windows** (without WSL): Ensure Docker is running. Clone the repo in Windows. Opening it in VS Code and using Dev Containers will spin up the container. The bind mount will be from a Windows path (which is slower, but the named volumes for heavy writes mitigate some of this). Make sure to enable file sharing for the drive in Docker settings. All commands are the same, but note that line endings issues are handled by using only Unix line endings in the repo (recommended).
* Windows host can’t directly run the ARM binaries; always use the container for that via QEMU. But the DevContainer abstracts that away.

**For macOS Developers:**

* Install Docker Desktop for Mac. Ensure it’s running.
* Install VS Code and the Dev Containers extension.
* Clone the repo (case-sensitive filesystem not required, but watch out if using APFS with case-insensitive, it should be fine).
* Open in VS Code container as usual. On Mac, the Docker backend uses a VM. Performance is generally good. The container image might run on linux/amd64 architecture (as our devcontainer likely is x86-based), which is fine on Intel Macs. On Apple Silicon (M1/M2), Docker can run multi-arch containers: it might pull an ARM64 base image. Our Dockerfile can work for both (if cross-compiling on an M1, we are in the odd situation of compiling from ARM64 to ARM64 – but still beneficial if target environment differs like glibc vs Darwin).

  * We explicitly test that scenario: on Apple M1, the container will likely be arm64 Debian. The cross-compiler then could potentially be skipped (since host arch matches target arch). However, for consistency (and because target might still differ in ABI), we treat it the same: use aarch64-linux-gnu-gcc to ensure code is linked against correct libraries (not macOS libs!). The QEMU step might not be needed on M1 if one wanted to run natively, but our flow will still use QEMU for uniformity (or we can detect and run binary directly on M1, but that’s optional).
* Develop as usual in container. On Apple Silicon, be mindful that QEMU user mode might actually not be needed (the binary could potentially run directly if statically linked for Darwin vs Linux differences – but since we link for Linux, we do need QEMU to emulate Linux syscalls even if CPU is same architecture).

**Common for All:** The devcontainer takes care of installing all needed compilers and libraries, so you should not need to install anything else on your host except Docker and VS Code. If you prefer not to use VS Code, you can run `docker-compose up -d && docker-compose exec dev bash` to use the environment with any editor, but you’ll miss out on some integrated debugging features.

### IT Setup Checklist (for onboarding teams)

If an IT department sets up developer machines or CI runners, here’s a checklist (could be automated via **Ansible** or scripts):

* **Hardware/OS:** Ensure the developer machine has sufficient resources for running Docker (at least 8GB RAM allocated to Docker, and \~20GB disk for images). Make sure virtualization is enabled (for Windows, BIOS Hyper-V setting on).
* **Software Installation:**

  * Docker Engine / Docker Desktop installed and tested (e.g. can run `hello-world` container).
  * For Windows: WSL2 enabled and Ubuntu (if using WSL) installed, or at least Docker Desktop with Linux containers.
  * Visual Studio Code installed. Extensions: *Dev Containers*, *Remote WSL* (for Windows), *Git* (for source control).
  * Git client installed and credentials managed (so devs can pull/push to repo).
* **Access:** If behind a proxy, configure Docker daemon with proxy settings and do the same in devcontainer (the Dockerfile will respect `http_proxy` if set at build time).
* **Clone Repository:** The team might maintain a central place or let each dev clone individually. Ensure they have access to the Git repository (SSH keys or credentials set up).
* **Ansible Playbook:** Optionally, an Ansible playbook is provided in `scripts/ansible-setup.yml` that automates many of these steps on Linux (installs Docker, adds user to group, installs VS Code via package manager, etc.). Running it on a fresh machine should prepare it for container-based dev. IT can customize it for their environment (like installing company-wide VPN or other tools as needed).
* **Testing the Setup:** After preparing, have a test where the new environment is used to run `make build` and `make test` to confirm everything is functioning. The CI pipeline’s success on a similar container is a good sign too.

This checklist ensures new developers can go from zero to productive with minimal manual steps. In the best case, they run one script (or IT does it for them) and then just start coding.

### Continuous Integration (CI) Integration Outline

We strive to keep CI as close to the dev environment as possible to avoid surprises. **GitHub Actions** is the preferred CI, using the same Docker setup to build and test:

* **Build and Test Workflow:** A GitHub Actions workflow YAML (e.g. `.github/workflows/ci.yml`) is included. On each push or pull request, it will:

  1. **Checkout code**.
  2. **Set up Docker** on the runner (on GitHub Ubuntu runners, Docker is available by default).
  3. **Build Dev Container** – We can either build the Docker image from the Dockerfile, or use the pre-built image from a registry (to save time). For first iteration, assume building it: `docker build -f .devcontainer/Dockerfile -t myproject-dev .`.
  4. **Run container and execute build/test** – e.g. `docker run --rm -v $(pwd):/workspace -v ccache:/home/vscode/.ccache myproject-dev /bin/sh -c "cmake --preset aarch64-release && cmake --build ... && ctest && cargo test ..."`. We might also use `docker-compose -f docker-compose.yml run dev make build && make test`.

     * The CI mounts the code and uses the same commands. Alternatively, since it’s single container, a simpler approach is just running all needed commands inside `docker run`.
  5. **Artifacts & Reports:** After building and running tests, the CI can gather artifacts:

     * The `bench_results.csv` and `report.html` can be uploaded as build artifacts for later analysis.
     * If tests produce a coverage report (e.g. an lcov info file or Cobertura XML), those can be uploaded or used in a follow-up job (for badge generation or coverage gating).
     * The compiled binaries could also be artifacts if needed (for QA or for deploying to a device).
  6. **Caching:** Use GitHub Actions cache for `~/.ccache` directory to speed up C++ rebuilds between runs. Similarly cache `~/.cargo/git` and `~/.cargo/registry` for Rust dependencies. Since we use Docker, some caches inside might be hidden, but we exposed ccache as a volume which can map to GH cache (one strategy: after build, tar the ccache volume or use `docker cp` to get its contents and cache that).
  7. **Matrix or Variants:** Optionally, set up matrix builds – for example, one matrix axis for build type (debug vs release) if we want to run tests in both, or even multi-arch build to ensure code also builds natively on x86 (if we want to compile & run unit tests on host arch for speed, in addition to cross). The primary target remains ARM though.
  8. **Status Badges:** The repo can include badges for build status and coverage. GH Actions will mark the status on PRs.

Using the devcontainer image in CI ensures that "it builds on my machine" equals "it builds on CI" because it's literally the same environment. This also future-proofs us: as new dependencies are added, we update Dockerfile once and both dev and CI get it.

* **Static Analysis & Linting:** The CI pipeline can include jobs for code quality:

  * C++: run `clang-tidy` or `cppcheck` on the codebase (the container could include these tools). Or include them in the build (CMake can integrate clang-tidy).
  * Python: run `flake8` or `pylint` on the Python code.
  * Rust: run `cargo clippy` for linting.
    These help maintain standards and catch issues early.
* **Automated Tests:** If we have unit tests, `ctest` will run them (possibly under QEMU if they are compiled for ARM; small unit tests under QEMU user should be fine). Python tests (via `pytest`) run on host architecture. Rust tests similarly can run via QEMU or some could be compiled for host for quickness.
* **Coverage Reporting:** We aim for traceability from features to tests. Thus, collecting test coverage is important:

  * **C++ Coverage:** We compile with `--coverage` (gcc flags) in a special CI build (maybe the debug build). Then run tests under QEMU. The `gcov` outputs (`.gcda` files) can be extracted and `lcov` used to gather coverage info. Then `genhtml` can produce an HTML report or we upload the info to a service like Codecov. This ensures we see which lines of code are tested.
  * **Python Coverage:** Use `pytest --cov` to get a coverage report for Python modules. This can output to an XML that can be combined or reported.
  * **Rust Coverage:** As of now, use `cargo tarpaulin` to run tests with coverage (tarpaulin uses an instrumented runtime to collect coverage on Linux; it might not support QEMU well, so for coverage we might run Rust tests on the host architecture using x86 build since logic is largely the same). Alternatively, use nightly Rust `-Zinstrument-coverage` with llvm-tools to collect data even on cross runs.
  * After collection, we could merge these coverages to understand total test coverage. At minimum, ensure each component’s coverage is high.

Traceability wise, we maintain a mapping from requirements or features to test cases (perhaps in documentation or a test plan). The CI coverage ensures that for each feature, relevant code is exercised. If a feature’s code isn’t hit by any test, that’s a gap. We encourage writing tests whenever a new feature is added (TDD/BDD approach).

### Standards and Best Practices

We adhere to various industry standards to keep the project maintainable and portable:

* **POSIX Compliance:** The C/C++ code targets POSIX.1-2008 standard for system calls and C library usage. This ensures it will run on any POSIX OS (Linux, etc.) and eases porting. We avoid non-portable Linux-only APIs unless absolutely necessary for performance (and even then, hide them behind conditional compilation).
* **DevContainer Best Practices:** Following Microsoft’s best practices, we keep the dev container config minimal yet effective. For example, we don’t run as root, we pre-install VS Code server to avoid delays, and we use docker-compose for clarity and extensibility. The container is also set up to auto-forward common ports (if our app opens ports) to the host for testing, via `devcontainer.json` settings.
* **CMake and Project Structure:** We use modern CMake (3.20+) with targets and interface libraries. We keep third-party dependencies to a minimum and include them cleanly (e.g., find\_package for ZeroMQ, or add\_subdirectory if we vendor something). The CMakePresets approach is recommended by Kitware for cross-compiling and for sharing common build setups in version control.
* **Coding Standards:** A style guide is in place (e.g., Google C++ style or LLVM style for C++; PEP8 for Python; Rust fmt for Rust). Formatters are integrated: clang-format config provided, `make format` to format C++ code; `rustfmt` for Rust; black or autopep8 for Python. The CI might enforce formatting (fail if diff).
* **Documentation:** Besides this design report, additional docs cover module-level details. We might use Doxygen for C++ API docs and doc comments in Rust which can be generated with `cargo doc`. The markdown documentation (for onboarding, FAQs, etc.) lives in the `docs/` folder. We ensure it stays updated as the code evolves (maybe requiring documentation updates as part of pull requests for significant changes).
* **Open-Source Software (OSS) Usage:** All included components (ZeroMQ, SQLite, Protobuf, Zyre) are permissively licensed and widely used. We comply with their licenses (e.g., include NOTICE files as needed). Security-wise, these are well-audited libraries. We also plan for upgradability of these libraries (keeping an eye on their updates, CVEs, etc.).

### Observability, Logging, and Tracing

To effectively debug and maintain a distributed system, we built in **observability** from the start:

* **Unified Logging:** Each service (C++, Python, Rust) uses logging best practices. For C++, we might use `spdlog` or the `<chrono>` time + i/o to print timestamped logs. Python uses the `logging` module, Rust uses the `log` crate with an env\_logger. We configure logs to include component name and perhaps a correlation ID if one is present in messages. For instance, when a UI request comes in (with an ID), the Data Service logs "Request X received", then when publishing an event as a result, it logs that with "correlation X". This helps trace a single transaction across services.
* **Distributed Tracing (Future):** We note that using something like OpenTelemetry could allow cross-service traces. As a starting point, the scaffold can include unique IDs in Protobuf messages (like each message has a `uint64 trace_id`) which is copied through flows, enabling log correlation manually. Future integration with an OpenTelemetry collector is possible (if, for example, we deployed a Jaeger agent on the device).
* **Metrics:** Although not fully implemented in the scaffold, we mention where metrics could be collected. E.g., Data Service could record metrics like "cache hit rate" or "DB write latency" and expose them (maybe via a simple HTTP server or just log them periodically). If needed, a Prometheus client library could be integrated for more formal metrics.
* **UX traces:** From a user perspective, when a user interacts with the system (e.g., presses a button on the UI that triggers a request), we should be able to trace that through logs: UI logs "Button pressed -> send UpdateUserRequest", Data Service logs "Received UpdateUserRequest from UI", Python/Rust services log "Received pub event for user update", etc. By grepping the trace ID or user ID, a developer can reconstruct the sequence. We provide an example in docs showing a sample log trace across components for a sample action.
* **Real-time Monitoring:** In dev mode, one can run all components and see their combined output. The scaffold could include a tmux or foreman configuration to launch multiple services in one terminal for convenience (not mandatory, but sometimes provided for demo).
* **Error Handling:** All services are built to handle errors gracefully (e.g., if DB is unavailable, the Data Service returns an error message rather than crashing; if a Protobuf message is unrecognized (version mismatch), it logs a warning). This makes the system robust for testing different scenarios.

### Feature Completeness and Traceability

To ensure the project meets all requirements, we enforce a link between features and tests:

* Each high-level feature (or user story) is documented in a requirements file or tracking system (e.g., JIRA or a simple Markdown list).
* We label tests or commit messages with the feature ID they cover. For example, a test function might be named `test_update_user_request()` which clearly relates to the "Update User" feature.
* We maintain a **traceability matrix** (could be a table in docs) mapping features to one or more test cases. For instance:

  | Feature                  | Test Cases                          | Coverage (%)                   |
  | ------------------------ | ----------------------------------- | ------------------------------ |
  | Update user data         | test\_update\_user\_request (Py)    | 95% of code paths              |
  | Real-time notification   | test\_realtime\_pub\_sub (C++)      | 90% (through integration test) |
  | Service discovery (Zyre) | test\_service\_advertisement (Rust) | 80% (some manual steps)        |

  This gives confidence that for each feature, we have automated tests hitting those paths. Any feature without tests is marked for adding tests.
* The CI’s coverage reports assist here: if a feature’s corresponding code isn’t executed in tests, its coverage will be low, flagging incomplete testing.
* **Definition of Done:** We set a policy that no feature is "done" until it has at least one automated test verifying it. Because our scaffold encourages cross-language interactions, many tests are integration tests (e.g., spin up DataService under QEMU, then have a Python test send a request and verify the response and published event). We provide an example integration test script in the repo to guide writing new ones.
* **Manual Testing & UX:** For features that involve UI (like an Android app in the architecture diagram), we outline how to manually test in a dev environment (perhaps using an Android emulator or a simple Python UI script). This is more for completeness – the scaffold itself is headless, but hooks are in place for UI to connect via ZeroMQ.

The scaffold not only sets up a project structure but also instills best practices in testing and quality.


