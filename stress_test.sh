#!/bin/bash

# --- Configuration ---
BASE_PATH="${HOME}/.kvdb"
SOCKET_PATH="${BASE_PATH}/db.sock"
LOG_FILE="${BASE_PATH}/daemon.log"

# Number of operations for each test phase
NUM_PUTS=10000
NUM_GETS=10000
NUM_DELS=10000
NUM_MIXED_OPS=10000 # For the mixed operation phase

# Number of concurrent processes (clients) to simulate
NUM_CONCURRENT_CLIENTS=1

# Length of random strings for values
VALUE_LENGTH=50

# --- Helper Functions ---

# Function to generate a random alphanumeric string
generate_random_string() {
    head /dev/urandom | LC_ALL=C tr -dc A-Za-z0-9_ | head -c ${1:-30} # Default length 30
}

# Function to check if db_main is running (rudimentary check)
check_daemon() {
    if ! pgrep -f "db_main" > /dev/null; then
        echo "Error: db_main daemon is not running. Please start it first."
        exit 1
    fi
}

# Function to clear the daemon log
clear_daemon_log() {
    echo "Clearing daemon log: ${LOG_FILE}"
    > "${LOG_FILE}"
}

# Function to run a CLI command safely
run_cli_command() {
    local command="$1"
    local output=$(echo "${command}" | nc -U "${SOCKET_PATH}")
    local exit_code=$?
    if [ $exit_code -ne 0 ]; then
        echo "CLI Error ($command): ${output}" >&2
    fi
}

# --- Stress Test Phases ---

# Phase 1: Pure PUT stress test
stress_puts() {
    echo "--- Starting PUT Stress Test (${NUM_PUTS} operations) ---"
    local start_time=$(date +%s)
    for i in $(seq 1 "${NUM_PUTS}"); do
        local key="key_$(generate_random_string 10)_${i}"
        local value="value_$(generate_random_string ${VALUE_LENGTH})"
        run_cli_command "put stress_key_${i} ${value}" &

        # Limit concurrent background jobs
        if (( i % NUM_CONCURRENT_CLIENTS == 0 )); then
            wait # Wait for NUM_CONCURRENT_CLIENTS jobs to finish
            echo -ne "PUT progress: $((i * 100 / NUM_PUTS))%\r"
        fi
    done
    wait # Wait for any remaining background jobs
    local end_time=$(date +%s)
    echo "" # Newline after progress
    local elapsed=$((end_time - start_time))
    puts_per_sec=$(echo "scale=2; $NUM_PUTS / $elapsed" | bc)
    secs_per_op=$(echo "scale=6; $elapsed / $NUM_PUTS" | bc)
    echo "PUT Stress Test completed in ${elapsed} seconds."
    echo "RESULT: $secs_per_op sec/op"
    echo "RESULT: $puts_per_sec ops/sec"    
}

# Phase 2: Pure GET stress test on existing keys
stress_gets() {
    echo "--- Starting GET Stress Test (${NUM_GETS} operations) ---"
    local start_time=$(date +%s)
    for i in $(seq 1 "${NUM_GETS}"); do
        # Try to GET keys that were put during the PUT phase
        local key="stress_key_$(( RANDOM % NUM_PUTS + 1 ))"
        run_cli_command "get ${key}" &

        if (( i % NUM_CONCURRENT_CLIENTS == 0 )); then
            wait
            echo -ne "GET progress: $((i * 100 / NUM_GETS))%\r"
        fi
    done
    wait
    local end_time=$(date +%s)
    echo ""
    local elapsed=$((end_time - start_time))
    gets_per_sec=$(echo "scale=2; $NUM_GETS / $elapsed" | bc)
    secs_per_op=$(echo "scale=6; $elapsed / $NUM_GETS" | bc)
    echo "GET Stress Test completed in ${elapsed} seconds."
    echo "RESULT: $secs_per_op sec/op"
    echo "RESULT: $gets_per_sec ops/sec"

}

# Phase 3: Pure DEL stress test
stress_dels() {
    echo "--- Starting DEL Stress Test (${NUM_DELS} operations) ---"
    local start_time=$(date +%s)
    for i in $(seq 1 "${NUM_DELS}"); do
        local key_to_del="stress_key_$(( RANDOM % NUM_PUTS + 1 ))" # Pick from existing range
        run_cli_command "del ${key_to_del}" &

        if (( i % NUM_CONCURRENT_CLIENTS == 0 )); then
            wait
            echo -ne "DEL progress: $((i * 100 / NUM_DELS))%\r"
        fi
    done
    wait
    local end_time=$(date +%s)
    echo ""
    local elapsed=$((end_time - start_time))
    dels_per_sec=$(echo "scale=2; $NUM_DELS / $elapsed" | bc)
    secs_per_op=$(echo "scale=6; $elapsed / $NUM_DELS" | bc)
    echo "DEL Stress Test completed in ${elapsed} seconds."
    echo "RESULT: $secs_per_op sec/op"
    echo "RESULT: $dels_per_sec ops/sec"
}

# Phase 4: Mixed PUT/GET/DEL operations
stress_mixed_operations() {
    echo "--- Starting Mixed Operations Stress Test (${NUM_MIXED_OPS} operations) ---"
    local start_time=$(date +%s)
    for i in $(seq 1 "${NUM_MIXED_OPS}"); do
        local op_type=$(( RANDOM % 3 )) # 0 for PUT, 1 for GET, 2 for DEL
        local key="mixed_key_$(generate_random_string 10)_${i}" # Use new keys for mixed
        local existing_key="stress_key_$(( RANDOM % NUM_PUTS + 1 ))" # Keys from previous PUTs

        if [ "$op_type" -eq 0 ]; then
            # PUT operation
            local value="mixed_value_$(generate_random_string ${VALUE_LENGTH})"
            run_cli_command "put ${key} ${value}" &
        elif [ "$op_type" -eq 1 ]; then
            # GET operation
            run_cli_command "get ${existing_key}" &
        else
            # DEL operation
            run_cli_command "del ${existing_key}" &
        fi

        if (( i % NUM_CONCURRENT_CLIENTS == 0 )); then
            wait
            echo -ne "Mixed progress: $((i * 100 / NUM_MIXED_OPS))%\r"
        fi
    done
    wait
    local end_time=$(date +%s)
    echo ""
    echo "Mixed Operations Stress Test completed in $((end_time - start_time)) seconds."
}

# --- Main Script Execution ---

echo "Starting Database Stress Test..."
echo "Socket: ${SOCKET_PATH}"
echo "Log File: ${LOG_FILE}"
echo "Concurrent Clients: ${NUM_CONCURRENT_CLIENTS}"

check_daemon
clear_daemon_log

echo "Press Enter to start tests. (Ensure db_main is running: ps aux | grep db_main)"
read -r

# Run the test phases sequentially
stress_puts
stress_gets
stress_dels
stress_mixed_operations

echo "Stress Test Completed!"
echo "--- Post-Test Checks ---"
echo "1. Examine daemon.log for errors:"
echo "   tail -n 50 ${LOG_FILE}"
echo "2. Check system resources (CPU, Memory) during the test (e.g., using htop or top)."
echo "3. Consider running tests multiple times or with higher NUM_ variables."
echo "4. To reset test, consider clearing the database first:"
echo "   rm -rf ~$HOME/.kvdb"