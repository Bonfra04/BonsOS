pipe=$1

# Ensure all pipes exists
if [ ! -p $pipe.in ]; then mkfifo $pipe.in; fi
if [ ! -p $pipe.out ]; then mkfifo $pipe.out; fi

pipe_read() {
    cat $pipe.out
}

pipe_write() {
    while read line; do
        echo $line > $pipe.in
    done
}

set -m # Enable Job Control

# order matters
pipe_write &
pipe_read &

# Wait for all functions to end
while [ 1 ]; do fg >/dev/null; [ $? == 1 ] && break; done