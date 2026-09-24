# SpeedTest

Command-line internet speed tester written in C.
SpeedTest uses libcurl for HTTP transfers and cJSON to read location data and
the bundled server list. It reports download and upload throughput in Mbps,
elapsed time in seconds, and transferred bytes.

## Build

Tools that is used here:

- GCC and Make.
- libcurl headers and library.
- cJSON headers and library.
- getops headers and library.

On Ubuntu or Debian, install them with:

```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev libcjson-dev
```

From the repository directory, build the executable:

```sh
make
```

The build creates `speedtest` and object files under `build/`. To remove
build files run:

``` sh
make clean
```

Run commands from the repository directory: server selection reads
`speedtest_server_list.json` from the current working directory.

## Usage

```sh
# Detect your location, select a server, then test download and upload
./speedtest -a

# Run a download or upload test using a server in a chosen country
./speedtest -d Lithuania
./speedtest -u Ukraine

# Display the location detected from your public IP address
./speedtest -l

# Detect your location and display the selected server
./speedtest -s

# Display command-line help
./speedtest -h
```

| `-a` | Select a server automatically, then run download and upload tests. |
| `-d <country>` | Select a reachable server in the country and test download speed. |
| `-u <country>` | Select a reachable server in the country and test upload speed. |
| `-l` | Look up and display your country and city. |
| `-s` | Look up your location and display a matching reachable server. |
| `-h` | Print manual for the program. |


Multiple action flags run in this fixed order: `-a`, `-d`, `-u`, `-l`, `-s`.
For example, `./speedtest -d Lithuania -u Lithuania` runs both tests, selecting
a server separately for each. Adding `-h` prints help before any requested
actions.

The exit status is `0` when all requested actions succeed and `1` when an action
or command-line validation fails. Running without a flag returns `1`.

## How it works

### Location and server selection

Automatic selection requests location data from `https://ipwho.is/`, then
searches in the server list for a matching country and city. It selects
the first reachable match in file order. If no city match is reachable, it uses the nearest country server.

A reachability check sends an HTTP HEAD request to the server host, and requires a successful 2xx response. 

### Download and upload measurements

Each transfer test targets a 15-second measurement window:

- Download repeatedly requests `/speedtest/random4000x4000.jpg`, counts received bytes, and discards the response data.
- Upload repeatedly posts a generated 1 MiB payload to `/speedtest/upload.php`.
  Only complete uploads with successful HTTP responses count toward the result.
  An unfinished request at the end of the window is excluded from uploaded bytes.

The reported speed is calculated as:

```text
Mbps = transferred_bytes × 8 / (elapsed_seconds × 1,000,000)
```

Transfers run sequentially, using one request at a time. An automatic run takes
about 30 seconds for successful transfers, plus location lookup and server
selection.

Results reflect throughput to the selected server during the run. Server load,
network conditions, and the sequential transfer method affect the measurement.
Duration, payload size, and endpoint paths are defined.

## Server list

`speedtest_server_list.json` contains an array of server records. Such as:

```json
[
  {
    "country": "Lithuania",
    "city": "Vilnius",
    "provider": "Example provider",
    "host": "speedtest.example.com:8080"
  }
]
```

The loader skips records without a string `host`. Missing or non-string
`country`, `city`, and `provider` fields default to `Unknown`. Use a host with an optional port and an
optional `http://` or `https://` scheme; transfer paths are appended by the program.

## Source layout

|
| `src/main.c` | Command-line parsing and action orchestration. |
| `src/location.c` | Public-IP location lookup. |
| `src/server.c` | Server-list loading, reachability checks, and selection. |
| `src/download.c` | Download measurement and result output. |
| `src/upload.c` | Upload payload generation, measurement, and result output. |
| `src/utils.c` | Buffers, timing, URL construction, and Mbps calculation. |
| `include/` | Shared structures and function declarations. |
| `Makefile` | Build and cleanup targets. |
