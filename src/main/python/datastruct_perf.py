#!/usr/bin/python

"""
Python code to test performance of different implementations of the indismo simulator.
"""

import sys
import os
import argparse
import subprocess
import json
import itertools
import datetime
import shutil
import xml.etree.cElementTree as ET

# Function that runs the indismo simulator.
def runIndismo(path, num_days, rng_seed, seeding_rate, r0, population_file, immunity_rate, output_prefix, disease_config_file, generate_person_file, environment={}):
    
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


def runExperiments(config_file):
    # Load json files
    config = json.load(open(config_file, 'r'))

    # Load the experiment configurations from the json
    experiments=[config['threads'], config['rng_seeds'], config['seeding_rates'], config['r0'], config['population_files'],config['immunity_rates']]

    name_sep = '_'
    # Create (summary) output dir with dir for experiment output
    time_stamp=datetime.datetime.now().strftime("%m%d%H%M%S") # add '%Y to add year and '%f' for milisec
    file_tag = time_stamp+name_sep+config['output']

    output_dir=os.path.join('output',time_stamp+name_sep+config['output'])

    experiments_dirs=os.path.join(output_dir,'experiments')
    if not os.path.isdir(experiments_dirs):
        os.makedirs(experiments_dirs)

    # Open the aggregated output files
    output_file=open(os.path.join(output_dir,file_tag+'_output.csv'),'w')
    log_file=open(os.path.join(output_dir,file_tag+'_log.csv'),'w')
    
    # Copy the json configuration file
    new_config_file=open(os.path.join(experiments_dirs,'exp_config.json'),'w')
    new_config_file.write(open(config_file,'r').read())
    new_config_file.close()
    
    # Create a copy of the environment variables to modify and pass to indismo
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
        runIndismo(config['indismo_path'], config['days'], experiment[1], experiment[2], experiment[3], experiment[4], experiment[5], output_prefix, config['disease_config_file'],config['generate_person_file'], env)
        
        
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

    output_file.close()
    log_file.close()

    # process the output
    processOutput(output_dir, file_tag)


def main(argv):
    runExperiments('./config/config_ds_perf_influenza.json')
    runExperiments('./config/config_ds_perf_measles.json')


if __name__ == "__main__":
    main(sys.argv)
