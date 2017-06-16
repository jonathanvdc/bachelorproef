#!/bin/bash
function run-and-time {
    ./gtester --gtest_filter="*$1" | grep "OK" | grep -o "(.*)" | sed "s/(\(.*\) ms)/0:0:0:\1:0/g"
}

make install_test
pushd build/installed/bin
echo "# serial-map" > ../../../parallel_map_measurements.txt
for i in `seq 1 2`;
do
    run-and-time MapPerfSerial >> ../../../parallel_map_measurements.txt
done
echo "# parallel-map" >> ../../../parallel_map_measurements.txt
for i in `seq 1 2`;
do
    run-and-time MapPerfParallel >> ../../../parallel_map_measurements.txt
done
popd