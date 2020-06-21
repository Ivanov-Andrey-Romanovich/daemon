# Tasks for linux cours
Repository with my daemon
# How to run
    Run a terminal and enter following commands:
    $ gcc Daemon.c -o Daemon -lpthread - compile programm
    $ ./Daemon input.txt - run a programm
    $ kill -SIGUSR1 pid - to send a first signal to daemon
    $ kill -SIGUSR2 pid - to run commands from input.txt (changes will be shown in commandsoutp.txt).
    $ kill pid - just to finish the programm
#Changes
	Now daemon can take and run several commands from input.txt
