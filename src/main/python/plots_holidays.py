#!/usr/bin/python

"""
Python code using matplotlib to plot disease spread with weekends/holidays in simulator vs without those

Author: Elise Kuylen (2016)
"""

import sys
import numpy as np
import matplotlib.pyplot as plt
import csv


def getResultsDailyCases(filename):
    """
    Calculate # of new cases per day from csv file.
    """
    results = []
    
    with open(filename) as f:
        reader = csv.reader(f, delimiter =';')
        for row in reader:
            for col in row:
                results.append(int(col))

    results_a = results[2:]
    results_b = results[1:-1]

    results_tot = [0, 0]

    for i in range(len(results_a)):
        results_tot.append(results_a[i] - results_b[i])

    return results_tot


def getResultsTotalCases(filename):
    """
    Calculate total number of cases for each day from csv file.
    """
    results = []
    
    with open(filename) as f:
        reader = csv.reader(f, delimiter =';')
        for row in reader:
            for col in row:
                results.append(int(col))

    return results           


def main(argv):

    if len(argv) == 2:
        ##############################
        # get results from csv files #
        ##############################

        daily_no_holidays = getResultsDailyCases(argv[0])
        daily_holidays = getResultsDailyCases(argv[1])

        tot_no_holidays = getResultsTotalCases(argv[0])
        tot_holidays = getResultsTotalCases(argv[1])

        days = range(1,301)

        ############################
        # plot results daily cases #
        ############################

        plt.plot(days, daily_no_holidays, 'r')
        plt.plot(days, daily_holidays, 'b')

        #legend, title, labels, etc ...
        plt.title("Holidays vs No holidays: daily cases (measles)")
        plt.xlabel("Days")
        plt.ylabel("Daily cases")
        plt.legend(["No holidays/weekends", "With holidays/weekends"])

        plt.show()

        ############################
        # plot results total cases #
        ############################

        plt.plot(days, tot_no_holidays, 'r')
        plt.plot(days, tot_holidays, 'b')

        #legend, title, labels, etc ...
        plt.title("Holidays vs No holidays: total cases (measles)")
        plt.xlabel("Days")
        plt.ylabel("Total cases")
        plt.legend(["no holidays/weekends", "with holidays/weekends"], loc=4)

        plt.show()
    else:
        print "Usage: $ python plots_holidays.py <output file no holidays> <output file holidays>"


if __name__ == "__main__":
    main(sys.argv[1:])

