#!/usr/bin/python

"""
Create an age-based contact matrix in XML-format.

Author: Elise Kuylen (2016)
"""

import sys
import csv
import xml.etree.cElementTree as ET

def createUniform():
    """
    Create a matrix that has the same number of contacts for all adults (children x 2) and cluster types.
    """
    root = ET.Element("matrices")

    cluster_types = ["household", "home_district", "work", "school", "day_district"]

    for cluster_type in cluster_types:
        cluster_root = ET.SubElement(root, cluster_type)

        for part_age in range(0, 81):
            participant = ET.SubElement(cluster_root, "participant")
            ET.SubElement(participant, "age").text = str(part_age)
            contacts = ET.SubElement(participant, "contacts")
            for cnt_age in range(0, 81):
                contact = ET.SubElement(contacts, "contact")
                ET.SubElement(contact, "age").text = str(cnt_age)
                rate = 3
                # if both persons are under 18, the contact rate is doubled            
                if part_age < 18 and cnt_age < 18:
                    rate *= 2
                ET.SubElement(contact, "rate").text = str(rate)

    tree = ET.ElementTree(root)
    tree.write("contact_matrix_uniform.xml")


def createFromCSV(household_file, work_file, school_file, community_file, prefix=""):
    """
    Create contact matrices from files with diary-study results.
    """
    root = ET.Element("matrices")
    cluster_types = ["household", "home_district", "work", "school", "day_district"]
    for cluster_type in cluster_types:
        cluster_root = ET.SubElement(root, cluster_type)
        cluster_file = None
        if cluster_type == "household":
            cluster_file = household_file
        elif cluster_type == "work":
            cluster_file = work_file
        elif cluster_type == "school":
            cluster_file = school_file
        else:
            cluster_file = community_file

        with open(cluster_file, 'r') as f:
            part_age = 0

            reader = csv.DictReader(f, delimiter=';')
            for row in reader:
                # make participant element
                participant = ET.SubElement(cluster_root, "participant")
                ET.SubElement(participant, "age").text = str(part_age)
                contacts = ET.SubElement(participant, "contacts")
                for cnt_age in range(0, 81):
                    # make contact element
                    contact = ET.SubElement(contacts, "contact")
                    ET.SubElement(contact, "age").text = str(cnt_age)
                    # get number of contacts
                    key = "age" + str(cnt_age)
                    rate = float(row[key].replace(',','.'))
                    if (cluster_type == "home_district") or (cluster_type == "day_district"):
                        rate /= 2
                    ET.SubElement(contact, "rate").text = str(rate)
                part_age += 1

    tree = ET.ElementTree(root)
    output_file = prefix + "_contact_matrix.xml";
    tree.write(output_file)


def main(argv):
    if len(argv) == 5:
        crateFromCSV(argv[0], argv[1], argv[2], argv[3], argv[4])
    elif len(argv) == 4:
        print "Creating contact matrix from CSV files."
        createFromCSV(argv[0], argv[1], argv[2], argv[3])
    else:
        print "Creating uniform contact matrix."
        createUniform()


if __name__ == "__main__":
    main(sys.argv[1:])
