#!/usr/bin/python

"""
Python code using matplotlib to plot timing results of simulations of indismo.

Author: Elise Kuylen (2015)
"""

import sys
import numpy as np
import matplotlib.pyplot as plt
import csv

def resultsOneR0(filename_basic, filename_loops, filename_cluster, filename_combo):
    """
    Calculate average run times with one R0.
    """

    times_basic = []
    times_loops = []
    times_cluster = []
    times_combo = []

    with open(filename_basic) as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        for row in reader:
            times_basic.append(int(row['run_time']))

    with open(filename_loops) as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        for row in reader:
            times_loops.append(int(row['run_time']))

    with open(filename_cluster) as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        for row in reader:
            times_cluster.append(int(row['run_time']))

    with open(filename_combo) as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        for row in reader:
            times_combo.append(int(row['run_time']))
    
    return [np.mean(times_basic), np.mean(times_loops), np.mean(times_cluster), np.mean(times_combo)]


def main(argv):

    if len(argv) == 8:
        # Get results to plot
        results_infl = resultsOneR0(argv[0], argv[1], argv[2], argv[3])
        results_meas = resultsOneR0(argv[4], argv[5], argv[6], argv[7])

        # Plot
        # Inspired in part by http://matplotlib.org/examples/api/barchart_demo.html
        N = 2
        
        ind = np.arange(N)      # the x locations for the groups
        width = 0.35            # the width of the bars

        fig, ax = plt.subplots()
        rects1 = ax.bar(ind, results_infl, width, color='r')
        rects2 = ax.bar(ind + width, results_meas, width, color='y')

        # Add some text for labels, title and axes ticks.
        ax.set_ylabel('Runtime (milliseconds)')
        ax.set_title('Comparision of runtimes')
        ax.set_xticks(ind + width)
        ax.set_xticklabels(('current', 'updated loops'), rotation=45)

        ax.legend((rects1[0], rects2[0]), ('Influenza - R0 = 1.1', 'Measles - R0 = 12'))


        def autolabel(rects):
            # attach some text labels
            for rect in rects:
                height = rect.get_height()
                ax.text(rect.get_x() + rect.get_width()/2., 1.05*height, '%d' % int(height), ha='center', va='bottom')

        autolabel(rects1)
        autolabel(rects2)

        plt.show()

    else:
        print "Usage: $ python plotTimes.py <filename basic influenza> <filename loops influenza> <filename cluster influenza> <filename combo influenza>"
                    "<filename basic measles> <filename loops measles> <filename cluster measles> <filename combo measles>"


if __name__ == "__main__":
    main(sys.argv[1:])

