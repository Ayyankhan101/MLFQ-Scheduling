#!/bin/bash

echo "Building MLFQ Scheduler with Web GUI support..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure and build
cmake ..
make

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo ""
    echo "To run with Web GUI:"
    echo "1. ./mlfq_scheduler"
    echo "2. Select option 5 (Web GUI)"
    echo "3. Open http://localhost:8080 in your browser"
    echo ""
    echo "Features of the Web GUI:"
    echo "• Professional modern design with animations"
    echo "• Real-time queue visualization"
    echo "• Interactive process management"
    echo "• Performance metrics dashboard"
    echo "• Step-by-step simulation control"
    echo "• Responsive design for all screen sizes"
else
    echo "Build failed!"
    exit 1
fi
