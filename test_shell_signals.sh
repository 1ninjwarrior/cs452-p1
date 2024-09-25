#!/bin/bash

echo "Testing shell signal handling"
echo "-----------------------------"

# Set the path to your shell executable
SHELL_EXECUTABLE="./myprogram"  # Update this to the correct path

# Check if the shell executable exists
if [ ! -f "$SHELL_EXECUTABLE" ]; then
    echo "Error: Shell executable not found at $SHELL_EXECUTABLE"
    exit 1
fi

# Start your shell
$SHELL_EXECUTABLE &
SHELL_PID=$!

sleep 1

# Test SIGINT (Ctrl+C)
echo "Testing SIGINT (Ctrl+C)"
kill -SIGINT $SHELL_PID
sleep 1

# Test SIGQUIT (Ctrl+\)
echo "Testing SIGQUIT (Ctrl+\)"
kill -SIGQUIT $SHELL_PID
sleep 1

# Test SIGTSTP (Ctrl+Z)
echo "Testing SIGTSTP (Ctrl+Z)"
kill -SIGTSTP $SHELL_PID
sleep 1

echo "Shell should still be running. Sending commands..."

# Test running a command and sending signals
echo "sleep 10" > /dev/tty
sleep 1
echo "Sending SIGINT to sleep command"
pkill -SIGINT sleep
sleep 2

echo "cat" > /dev/tty
sleep 1
echo "Sending SIGINT to cat command"
pkill -SIGINT cat
sleep 2

# Clean up
kill $SHELL_PID

echo "Test complete. Check the output to verify correct behavior."