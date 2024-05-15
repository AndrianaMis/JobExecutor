#!/bin/bash

#Προσπαθεί να πάρει το jobID από αυτό που επιστρέφει η εργασία poll running και μετά να κάνει stop job_id
# Εργασίες που τρέχουν
./jobCommander poll running | awk '{print $2}' | while read -r job_id; do
    ./jobCommander stop "$job_id"
done

# Εργασίες σε αναμονή 
./jobCommander poll queued | awk '{print $2}' | while read -r job_id; do
    ./jobCommander stop "$job_id"
done
