# Fuzzkrieg

A coverage-guided fuzzer for the iOS XNU kernel, specifically targeting iOS 18.

## Features

- Coverage-guided fuzzing using libimobiledevice
- Advanced mutation strategies for kernel structures
- Crash analysis and reporting
- Test case minimization
- Parallel fuzzing support
- iOS 18 specific optimizations

## Requirements

### macOS Dependencies

```bash
# Install Homebrew if you haven't already
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required dependencies
brew install libimobiledevice
brew install libplist
brew install libusb
brew install libimobiledevice-glue
```

### iOS Device Requirements

- iOS 18 device or simulator
- Developer mode enabled
- Trusted computer status
- Valid provisioning profile

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/fuzzkrieg.git
cd fuzzkrieg

# Build the project
make clean
make
```

## Usage

### Basic Usage

```bash
# Run fuzzer with default settings
./bin/fuzzkrieg

# Run with specific test case
./bin/fuzzkrieg --testcase path/to/testcase

# Run with custom configuration
./bin/fuzzkrieg --workers 4 --timeout 3600
```

### Command Line Options

- `--testcase`: Specify a test case file
- `--workers`: Number of parallel workers (default: 4)
- `--timeout`: Fuzzing timeout in seconds
- `--log`: Specify log file location
- `--coverage`: Enable coverage tracking
- `--minimize`: Enable test case minimization

### Crash Analysis

Crashes are stored in the `crashes/` directory:

- `crashes/logs/`: Raw crash logs
- `crashes/testcases/`: Minimized test cases
- `crashes/ios18/`: iOS 18 specific crash reports

## Project Structure

```
fuzzkrieg/
├── include/           # Header files
├── src/              # Source files
│   ├── analysis/     # Crash analysis
│   ├── fuzzer/       # Core fuzzing logic
│   ├── mutators/     # Mutation strategies
│   └── testcase/     # Test case management
├── bin/              # Compiled binaries
├── obj/              # Object files
└── crashes/          # Crash reports and logs
```
