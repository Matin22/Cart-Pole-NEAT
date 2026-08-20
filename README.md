# Cartpole Pendulum NEAT

A cart-pole (inverted pendulum) balancing simulation where a neural network controller is evolved from scratch using [NEAT](http://nn.cs.utexas.edu/downloads/papers/stanley.ec02.pdf) (NeuroEvolution of Augmenting Topologies). Built in C++23 with SFML for real-time visualization.

The cart can be driven manually, or handed over to the best genome produced by evolutionary training, which balances the pole by learning both the network's weights and topology.

## How it works

- On first launch, if no saved genome is found at `res/best_genome.bin`, the NEAT algorithm evolves a population of networks against the cart-pole physics simulation until a fit controller emerges, then saves it.
- On subsequent launches, the saved genome is loaded instantly and used to control the cart.
- The controller receives the cart's normalized position/velocity and the pole's angle (as sin/cos) and angular velocity, and outputs a single acceleration command.

## Controls

| Key | Action |
| --- | --- |
| `A` / `D` | Move the cart left / right (manual mode) |
| `R` | Reset the cart and pendulum |
| `T` | Toggle AI control on/off |
| `Esc` / `Q` | Quit |

## Building

Requires [CMake](https://cmake.org/download/) 3.28+ and a C++23 compiler. SFML 3 is fetched automatically via CMake's `FetchContent`.

```sh
cmake -B build
cmake --build build
```

The executable is placed in `build/bin/`, with the `res/` folder copied alongside it.

## Project layout

- `src/environment/`: cart and pendulum physics, rendering, input handling
- `src/agent/`: NEAT implementation (genomes, mutation, speciation, evolution, forward pass, training loop)
- `src/config.hpp` / `src/neatConfig.hpp` — simulation and NEAT hyperparameters

## Credits

This project builds on ideas and prior art from:

- [johnBuffer/Pendulum-NEAT](https://github.com/johnBuffer/Pendulum-NEAT): the original cart-pole NEAT project this one draws inspiration from.

## License

MIT. See [LICENSE.md](LICENSE.md).
