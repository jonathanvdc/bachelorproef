#!/usr/bin/python

"""
Create .csv files from logfiles produced by the simulator.

Author: Elise Kuylen (2016)
"""

import sys
import csv
import random

def prepare_csv_transmissions(log_file, csv_file):
    """
    Create .csv file from logfile with only transmissions. 
    """

    with open(csv_file, 'w') as c:

        c_fieldnames = ['person_id', 'begin_infection']
        c_writer = csv.DictWriter(c, fieldnames=c_fieldnames)
        c_writer.writeheader()
            
        with open (log_file, 'r') as f:
            for line in f:
                # remove logging info
                line = line[50:]
                # check if [PART] or [CONT]
                identifier = line[:14]
                line = line[15:]
                line = line.split()

                dic = {}
                dic['person_id'] = line[1]
                dic['begin_infection'] =line[4]
                c_writer.writerow(dic)


def prepare_csv(log_file_path, participants_file='participants.csv', contacts_file='contacts.csv'):
    """
    From logfile with all contacts, logged in the following format:
    [PART] local_id part_age part_gender
    [CONT] local_id part_age cnt_age cnt_home cnt_work cnt_school cnt_other sim_day
    Create csv-files Participants.csv and Contacts.csv
    """

    # Open csv files to write to.
    with open(log_file_path+'_'+participants_file, 'w') as p, open(log_file_path+'_'+contacts_file, 'w') as c:

        # Write headers for csv files
        p_fieldnames = ['local_id', 'part_age', 'part_gender']
        p_writer = csv.DictWriter(p, fieldnames=p_fieldnames)
        p_writer.writeheader()
        
        c_fieldnames = ['local_id', 'part_age', 'cnt_age', 'cnt_home', 'cnt_work', 'cnt_school', 'cnt_other', 'sim_day']
        c_writer = csv.DictWriter(c, fieldnames=c_fieldnames)
        c_writer.writeheader()
                     
        with open (log_file_path+'_logfile.txt', 'r') as f:
            for line in f:
                ## remove logging info
                # line = line[50:]
                ## check if line-tag is [CONT]
                identifier = line[:6]
                line = line[7:]
                line = line.split()
                if identifier == "[PART]":
                    dic = {}
                    for i in range(len(p_fieldnames)):
                        value = line[i]
                        dic[p_fieldnames[i]] = value
                    p_writer.writerow(dic)
                # else for Contacts.csv
                else:
                    dic = {}
                    for i in range(len(c_fieldnames)):
                        value = line[i]
                        dic[c_fieldnames[i]] = value
                    c_writer.writerow(dic)

def main(argv):
    if len(argv) == 1:
        prepare_csv(argv[0])
    
    elif len(argv) == 2:
        prepare_csv(argv[0], argv[1])
    else:
        print ("Usage: python prepare_cnt_csv.py <logfile.txt> [<participants_file.csv>] [<contacts_file.csv>]")


if __name__ == "__main__":
    main(sys.argv[1:])

