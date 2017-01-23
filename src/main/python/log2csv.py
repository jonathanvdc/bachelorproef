#!/usr/bin/python

#############################################################################
#  This file is part of the Stride software. 
#  It is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or any 
#  later version.
#  The software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License,
#  along with the software. If not, see <http://www.gnu.org/licenses/>.
#  see http://www.gnu.org/licenses/.
#
#  Copyright 2016, Willem L, Kuylen E & Broeckhove J
#############################################################################

"""
Create .csv files from logfiles produced by the simulator.

Author: Elise Kuylen (2016)
"""

import sys
import csv
import random

def prepare_csv(log_file_path):
    """
    From logfile with all contacts, logged in the following format:
    [PART] local_id part_age part_gender
    [CONT] local_id part_age cnt_age cnt_home cnt_work cnt_school cnt_other sim_day
    Create csv-files Participants.csv and Contacts.csv
    """

    participants_file = log_file_path + '_participants.csv'
    contacts_file     = log_file_path + '_contacts.csv'
    transmission_file = log_file_path + '_transmissions.csv'
    
    # Open csv files to write to.
    with open(participants_file, 'w') as p, open(contacts_file, 'w') as c, open(transmission_file,'w') as t:

        # Write headers for csv files
        p_fieldnames = ['local_id', 'part_age', 'part_gender']
        p_writer = csv.DictWriter(p, fieldnames=p_fieldnames)
        p_writer.writeheader()
        
        c_fieldnames = ['local_id', 'part_age', 'cnt_age', 'cnt_home', 'cnt_work', 'cnt_school', 'cnt_other', 'sim_day']
        c_writer = csv.DictWriter(c, fieldnames=c_fieldnames)
        c_writer.writeheader()
        
        t_fieldnames = ['person_id', 'begin_infection']
        t_writer = csv.DictWriter(t, fieldnames=t_fieldnames)
        t_writer.writeheader()
        
        with open (log_file_path+'_logfile.txt', 'r') as f:
            for line in f:
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
                if identifier == "[CONT]":
                    dic = {}
                    for i in range(len(c_fieldnames)):
                        value = line[i]
                        dic[c_fieldnames[i]] = value
                    c_writer.writerow(dic)
                # else for transmissions
                if identifier == "[TRAN]":
                    dic = {}
                    for i in range(len(t_fieldnames)):
                        value = line[i]
                        dic[t_fieldnames[i]] = value
                    c_writer.writerow(dic)

def main(argv):
    if len(argv) == 1:
        prepare_csv(argv[0])
    else:
        print ("Usage: python prepare_csv.py <run_file_path>")


if __name__ == "__main__":
    main(sys.argv[1:])

