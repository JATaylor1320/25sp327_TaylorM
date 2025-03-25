#!/bin/bash
#send_signal.sh
echo "Sending SIGTSTP to child process (PID: $(pgrep child)) every 3 seconds...">&1
pid=$(pgrep child)
count=0
while [ $count -lt 5 ]; do
    if [ -n "$pid" ]; then
        kill -SIGTSTP $pid
        echo "Sending SIGTSTP to $pid"
    fi
    sleep 3
    count=$((count + 1))
done

echo "Done sending signals. Now sending SIGTERM to child."
if [ -n "$pid" ]; then
    kill -SIGTERM $pid
fi
