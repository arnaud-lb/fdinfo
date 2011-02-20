
CLI tool to display the state of a running program's file descriptors (size, position, bytes per second, ...), when the program does not provide such informations by itself.

Requirements: A Linux kernel

Use case:

    # dd if=/dev/somedevice of=/dev/otherdevice &
    # fdinfo $!
        0:   163 MiB /     0 B   [    0 B   / s] /dev/somedevice
        1:     0 B   /     0 B   [    0 B   / s] /dev/otherdevice
        2:     0 B   /     0 B   [    0 B   / s] /dev/pts/6
        0:   212 MiB /     0 B   [   48 MiB / s] /dev/somedevice
        1:     0 B   /     0 B   [    0 B   / s] /dev/otherdevice
        2:     0 B   /     0 B   [    0 B   / s] /dev/pts/6
        0:   260 MiB /     0 B   [   47 MiB / s] /dev/somedevice
        1:     0 B   /     0 B   [    0 B   / s] /dev/otherdevice
        2:     0 B   /     0 B   [    0 B   / s] /dev/pts/6
        0:   305 MiB /     0 B   [   45 MiB / s] /dev/somedevice
        1:     0 B   /     0 B   [    0 B   / s] /dev/otherdevice
        2:     0 B   /     0 B   [    0 B   / s] /dev/pts/6
        0:   350 MiB /     0 B   [   44 MiB / s] /dev/somedevice
        ...


