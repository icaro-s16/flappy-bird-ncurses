# Flappy Bird in Ncurses

A classic Flappy Bird game clone implemented in C, running directly in your terminal using the ncurses library.

## Getting Started

Follow these instructions to get a copy of the project up and running on your local machine.

### Prerequisites

This project requires the **ncurses** development library to be installed on your system. This library allows the game to draw graphics and handle input in the terminal.

You can install it using your system's package manager:

* **On Fedora, CentOS, or RHEL:**
    ```bash
    sudo dnf install ncurses-devel
    ```
* **On Debian, Ubuntu, or derivatives:**
    ```bash
    sudo apt-get install libncurses-dev
    ```

### Compiling

Once the dependency is installed, you can compile the project using `gcc`. Navigate to the project directory and run the following command:

```bash
gcc flappy_bird.c -o flappy_bird -lm -lncurses
