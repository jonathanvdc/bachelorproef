#!/usr/bin/python

"""
Python code using matplotlib to plot timing results of simulator runs.

Author: Elise Kuylen (2015)
"""

import sys
import numpy as np
import matplotlib.pyplot as plt
import csv

def resultsMultipleR0(filename_basic, filename_loops):
    """
    Calculate results from output files.
    """

    times_basic = {}
    times_loops = {}

    with open(filename_basic) as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        for row in reader:
            r0 = float(row['R0'])
            if r0 in times_basic:
                times_basic[r0].append(int(row['run_time']))
            else:
                times_basic[r0] = [int(row['run_time'])]

    with open(filename_loops) as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        for row in reader:
            r0 = float(row['R0'])
            if r0 in times_loops:
                times_loops[r0].append(int(row['run_time']))
            else:
                times_loops[r0] = [int(row['run_time'])]

    for key in times_basic:
        times_basic[key] = np.mean(times_basic[key])

    for key in times_loops:
        times_loops[key] = np.mean(times_loops[key])

    return [times_basic, times_loops]


def main(argv):
    if len(argv) == 4:

        ###########################
        # Plot influenza results. #
        ###########################

        results_infl = resultsMultipleR0(argv[0], argv[1])
        results_infl_BASIC = []
        results_infl_LOOPS = []

        r0_infl = [1.1,1.25,1.4,1.8,3]

        for r0 in r0_infl:
            results_infl_BASIC.append(results_infl[0][r0])
            results_infl_LOOPS.append(results_infl[1][r0])

        # Plot
        # Inspired in part by http://matplotlib.org/examples/api/barchart_demo.html
        N = 5
        
        ind = np.arange(N)      # the x locations for the groups
        width = 0.35            # the width of the bars

        fig, ax = plt.subplots()
        rects1 = ax.bar(ind, results_infl_BASIC, width, color='r')
        rects2 = ax.bar(ind + width, results_infl_LOOPS, width, color='y')

        # Add some text for labels, title and axes ticks.
        ax.set_xlabel('R0')
        ax.set_ylabel('Runtime (milliseconds)')
        ax.set_title('Comparision of runtimes for influenza with multiple R0')
        ax.set_xticks(ind + width)
        ax.set_xticklabels(r0_infl, rotation=45)

        ax.legend((rects1[0], rects2[0]), ('Current', 'Updated loops'))
        
        plt.show()

        ######################## 
        # Plot measles results. #
        ########################

        results_meas = resultsMultipleR0(argv[2], argv[3])
        results_meas_BASIC = []
        results_meas_LOOPS = []

        r0_infl = [12,13.5,15,16.5,18]

        for r0 in r0_infl:
            results_meas_BASIC.append(results_meas[0][r0])
            results_meas_LOOPS.append(results_meas[1][r0])

        # Plot
        # Inspired in part by http://matplotlib.org/examples/api/barchart_demo.html
        N = 5
        
        ind = np.arange(N)      # the x locations for the groups
        width = 0.35            # the width of the bars

        fig, ax = plt.subplots()
        rects1 = ax.bar(ind, results_meas_BASIC, width, color='r')
        rects2 = ax.bar(ind + width, results_meas_LOOPS, width, color='y')

        # Add some text for labels, title and axes ticks.
        ax.set_xlabel('R0')
        ax.set_ylabel('Runtime (milliseconds)')
        ax.set_title('Comparision of runtimes for measles with multiple R0')
        ax.set_xticks(ind + width)
        ax.set_xticklabels(r0_infl, rotation=45)

        ax.legend((rects1[0], rects2[0]), ('Current', 'Updated loops'))
        
        plt.show()
    else:
        print "Usage: $ python plotTimes.py <filename basic influenza> <filename loops influenza> <filename basic measles> <filename loops measles>"

if __name__ == "__main__":
    main(sys.argv[1:])
