#!/usr/bin/python

import sys
import os
import argparse
import subprocess
import json
import itertools
import datetime
import shutil
import csv

import xml.etree.cElementTree as ET


# Function that runs the simulator.
def runIndismo(path, num_days, rng_seed, seeding_rate, r0, population_file, immunity_rate, output_prefix, disease_config_file, generate_person_file, num_participants_survey, environment={}):
    
    # Write configuration file
    root = ET.Element("run")
    
    ET.SubElement(root, "num_days").text = str(num_days)
    ET.SubElement(root, "rng_seed").text = str(rng_seed)
    ET.SubElement(root, "seeding_rate").text = str(seeding_rate)
    ET.SubElement(root, "r0").text = str(r0)
    ET.SubElement(root, "population_file").text = population_file
    ET.SubElement(root, "immunity_rate").text = str(immunity_rate)
    ET.SubElement(root, "output_prefix").text = str(str(output_prefix))
    ET.SubElement(root, "disease_config_file").text = str(str(disease_config_file))
    ET.SubElement(root, "generate_person_file").text = str(str(generate_person_file))
    ET.SubElement(root, "num_participants_survey").text = str(str(num_participants_survey))
    
    tree = ET.ElementTree(root)
    tree.write(str(output_prefix)+ ".xml")
    
    # Execute the call
    return subprocess.call([path, '--config_file', str(output_prefix)+ ".xml"], env=environment)


# Function that uses an R script to process the results
def processOutput(output_dir,file_tag):
    
    work_dir = os.getcwd()
    r_file=open(os.path.join(output_dir,'plot_timings.R'),'w')
    r_file.write('# R file to plot the output'+ '\n')
    r_file.write('\n' + '## SET R WORK DIR'+ '\n')
    r_file.write('setwd("' + os.path.join(work_dir,output_dir) + '")'+ '\n')
    r_file.write('\n' + '## LOAD HELP FUNCTIONS'+ '\n')
    r_file.write('source("' + work_dir + '/lib/plot_results_lib.R")'+ '\n')
    r_file.write('\n' + '## GET DATA'+ '\n')
    r_file.write('data_tag <- "' + file_tag + '"' + '\n')
    r_file.write('\n## PLOT ALL RESULTS'+ '\n')
    r_file.write('plot_all_results(data_tag)'+ '\n')
    r_file.close()
    cmd_r = 'Rscript ' + output_dir + '/plot_timings.R'
    # RUN Rscript
    os.system(cmd_r)

    work_dir = os.getcwd()
    r_file=open(os.path.join(output_dir,'score_contacts.R'),'w')
    r_file.write('# R file to score the contacts'+ '\n')
    r_file.write('\n' + '## SET R WORK DIR'+ '\n')
    r_file.write('setwd("' + os.path.join(work_dir,output_dir) + '")'+ '\n')
    r_file.write('\n' + '## LOAD HELP FUNCTIONS'+ '\n')
    r_file.write('source("' + work_dir + '/lib/score_cnt_data_lib.R")'+ '\n')
    r_file.close()
    cmd_r = 'Rscript ' + output_dir + '/score_contacts.R'
    # RUN Rscript
    os.system(cmd_r)


"""
    From logfile with all contacts, logged in the following format:
    [PART] local_id part_age part_gender
    [CONT] local_id part_age cnt_age cnt_home cnt_work cnt_school cnt_other sim_day
    Create csv-files Contacts.csv
    """

import sys
import csv
import random

def prepare_csv(log_file_path, participants_file='participants.csv', contacts_file='contacts.csv', transmission_file='transmission.csv'):
    with open(log_file_path+'_'+participants_file, 'w') as p, open(log_file_path+'_'+contacts_file, 'w') as c, open(log_file_path+'_'+transmission_file, 'w') as t:
        p_fieldnames = ['local_id', 'part_age', 'part_gender']
        p_writer = csv.DictWriter(p, fieldnames=p_fieldnames)
        p_writer.writeheader()
        
        c_fieldnames = ['local_id', 'part_age', 'cnt_age',
                        'cnt_home', 'cnt_work', 'cnt_school', 'cnt_other', 'sim_day']
        c_writer = csv.DictWriter(c, fieldnames=c_fieldnames)
        c_writer.writeheader()
        
        t_fieldnames = ['local_id', 'new_infected_id', 'cnt_location','sim_day']
        t_writer = csv.DictWriter(t, fieldnames=t_fieldnames)
        t_writer.writeheader()
            
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
                if identifier == "[CONT]":
                    dic = {}
                    for i in range(len(c_fieldnames)):
                        value = line[i]
                        dic[c_fieldnames[i]] = value
                    c_writer.writerow(dic)

                if identifier == "[TRAN]":
                    dic = {}
                    for i in range(len(t_fieldnames)):
                        value = line[i]
                        dic[t_fieldnames[i]] = value
                    t_writer.writerow(dic)



def main(argv):
    
    # Arguments parser
    parser = argparse.ArgumentParser(description='Script to execute multiple runs of the simulator.')
    parser.add_argument('--config', help='A config file describing the experiments to run.', default='./config/config_default.json', type=str)
    
    args = vars(parser.parse_args())
    
    # Load the json file
    config=json.load(open(args['config'], 'r'))
    
    # Load the experiment configurations from the json
    experiments=[config['threads'], config['rng_seed'], config['seeding_rate'], config['r0'], config['population_file'],config['immunity_rate']]
    
    # Check the config'output'... if not empty, use separator in names
    name_sep = ''
    if len(config['output_tag'])>0:
        name_sep='_'
    
    # Create (summary) output dir with dir for experiment output
    time_stamp=datetime.datetime.now().strftime("%m%d%H%M%S") # add '%Y to add year and '%f' for milisec
    file_tag = time_stamp+name_sep+config['output_tag']

    output_dir=os.path.join('output',time_stamp+name_sep+config['output_tag'])
    experiments_dirs=os.path.join(output_dir,'experiments')
    if not os.path.isdir(experiments_dirs):
        os.makedirs(experiments_dirs)
    
    # Open the aggregated output files
    output_file=open(os.path.join(output_dir,file_tag+'_output.csv'),'w')
    log_file=open(os.path.join(output_dir,file_tag+'_log.csv'),'w')

    # Copy the json configuration file
    config_file=open(os.path.join(experiments_dirs,'exp_config.json'),'w')
    config_file.write(open(args['config'],'r').read())
    config_file.close()
    
    
    # Create a copy of the environment variables to modify and pass to simulator
    env = os.environ.copy()
    
    is_first=True # needed to have only one header in the aggregated output
    
    
    # RUN ALL EXPERIMENTS
    # note: 'intertools.product' makes all combinations within 'experiments'
    # note: (a,b) is a tupule... enumerate(vect) gives ((1, vect_n1) (2,vect_n2) ...)
    for (index, experiment) in enumerate(itertools.product(*experiments)):
     
        # Create a output_prefix for the experiment output
        output_prefix=os.path.join(experiments_dirs,'exp'+str(index))
        
        
        # Set the OpenMP environment
        env['OMP_NUM_THREADS']=str(experiment[0])
        env['OMP_SCHEDULE']=str(config['omp_schedule'])
        
        # Run the simulator     ('experiment[0]' has been used for the OMP_NUM_THREADS)
        runIndismo(config['stride_path'], config['num_days'], experiment[1], experiment[2], experiment[3], experiment[4], experiment[5], output_prefix, config['disease_config_file'],config['generate_person_file'],config['num_participants_survey'], env)
        
        
        # Append the aggregated outputs
        if is_first:
            output_file.write(open(output_prefix+'_output.csv','r').read())
            is_first=False
        else:
            lines = open(output_prefix+'_output.csv','r').readlines()
            for line in lines[1:]:
                output_file.write(line)
        
        log_file.write(open(output_prefix+'_log.csv','r').read())
        log_file.flush()
        output_file.flush()

        # get contact and participant files
        prepare_csv(output_prefix)


    output_file.close()
    log_file.close()

    ## process the output
    processOutput(output_dir,file_tag)



if __name__ == "__main__":
    main(sys.argv)