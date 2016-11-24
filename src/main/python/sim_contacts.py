#!/usr/bin/python

"""
Python code for running & analysing simulation of all contacts between persons.

Author: Elise Kuylen
"""

from prepare_csv import prepare_csv
import datetime
import os
import sys

def main(argv):
    # create output directory
    if not os.path.isdir("./output"):
        os.mkdir("./output")

    # run simulator
    print("Running the simulator ...")
    config_file = "config/run_config_all_contacts.xml"
    os.system("bin/indismo -c "+config_file)

    # prepare csv files
    print("Preparing csv files ...")
    prepare_csv("logfile.txt", "./output/Participants.csv", "./output/Contacts.csv")

    # TODO run R script
    #os.system("Rscript lib/indismo_contact_matrices.R")

if __name__ == "__main__":
    main(sys.argv[1:])
